/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 */

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "MQTTClient.h"
#include "evrythng/evrythng.h"
#include "evrythng/platform.h"
#include "evrythng_tls_certificate.h"

#define TOPIC_MAX_LEN 128
#define USERNAME "authorization"

static void mqtt_thread(void* arg);
static void message_callback(MessageData* data, void* userdata);
static evrythng_return_t evrythng_connect_internal(evrythng_handle_t handle);
static evrythng_return_t evrythng_disconnect_internal(evrythng_handle_t handle, int gracefull);

typedef struct sub_callback_t {
    char*                   topic;
    int                     qos;
    sub_callback*           callback;
    struct sub_callback_t*  next;
} sub_callback_t;


enum { MQTT_NOP, MQTT_CONNECT, MQTT_DISCONNECT, MQTT_PUBLISH, MQTT_SUBSCRIBE, MQTT_UNSUBSCRIBE };
typedef struct mqtt_op 
{
    int op;
    const char* topic;
    MQTTMessage* message;
    sub_callback* callback;
    evrythng_return_t result;
} mqtt_op;


struct evrythng_ctx_t {
    char*   host;
    int     port;
    char*   client_id;
    char*   key;
    const char* ca_buf;
    size_t  ca_size;
    int     secure_connection;
    int     qos;
    int     initialized;
    int     command_timeout_ms;

    Thread  mqtt_thread;
    int     mqtt_thread_stop;
    int     mqtt_thread_priority;
    int     mqtt_thread_stacksize;

    unsigned char serialize_buffer[1024];
    unsigned char read_buffer[1024];

    evrythng_log_callback log_callback;

    evrythng_callback on_connection_lost;
    evrythng_callback on_connection_restored;

    Network     mqtt_network;
    MQTTClient  mqtt_client;
    MQTTPacket_connectData  mqtt_conn_opts;

    sub_callback_t *sub_callbacks;

    mqtt_op     next_op;
    Mutex       async_op_mtx;
    Mutex       next_op_mtx;
    Semaphore   next_op_ready_sem;
    Semaphore   next_op_result_sem;
};


static void evrythng_log(evrythng_handle_t handle, evrythng_log_level_t level, const char* fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    if (handle && handle->log_callback)
        handle->log_callback(level, fmt, vl);
    va_end(vl);
}
#define debug(fmt, ...) evrythng_log(handle, EVRYTHNG_LOG_DEBUG, fmt,  ##__VA_ARGS__);
#define warning(fmt, ...) evrythng_log(handle, EVRYTHNG_LOG_WARNING, fmt,  ##__VA_ARGS__);
#define error(fmt, ...) evrythng_log(handle, EVRYTHNG_LOG_ERROR, fmt,  ##__VA_ARGS__);


evrythng_return_t EvrythngInitHandle(evrythng_handle_t* handle)
{
    if (!handle) 
        return EVRYTHNG_BAD_ARGS;

    *handle = (evrythng_handle_t)platform_malloc(sizeof(struct evrythng_ctx_t));

    if (!*handle) 
        return EVRYTHNG_MEMORY_ERROR;

    memset(*handle, 0, sizeof(struct evrythng_ctx_t));
    memcpy(&(*handle)->mqtt_conn_opts, &(MQTTPacket_connectData)MQTTPacket_connectData_initializer, sizeof(MQTTPacket_connectData));

    (*handle)->mqtt_conn_opts.MQTTVersion = 3;
    (*handle)->mqtt_conn_opts.keepAliveInterval = 60;
    (*handle)->mqtt_conn_opts.cleansession = 1;
    (*handle)->mqtt_conn_opts.willFlag = 0;
    (*handle)->mqtt_conn_opts.username.cstring = USERNAME;
    (*handle)->qos = 1;

    (*handle)->ca_buf = cert_buffer;
    (*handle)->ca_size = sizeof cert_buffer;

    (*handle)->command_timeout_ms = (*handle)->mqtt_conn_opts.keepAliveInterval * 1000;

	MQTTClientInit(
            &(*handle)->mqtt_client, 
            &(*handle)->mqtt_network, 
            (*handle)->command_timeout_ms, 
            (*handle)->serialize_buffer, sizeof((*handle)->serialize_buffer), 
            (*handle)->read_buffer, sizeof((*handle)->read_buffer));

    (*handle)->mqtt_thread_stacksize = 8192;

    (*handle)->mqtt_client.messageHandler = message_callback;
    (*handle)->mqtt_client.messageHandlerData = (void*)(*handle);

    platform_mutex_init(&(*handle)->next_op_mtx);
    platform_mutex_init(&(*handle)->async_op_mtx);
    platform_semaphore_init(&(*handle)->next_op_ready_sem);
    platform_semaphore_init(&(*handle)->next_op_result_sem);

    return EVRYTHNG_SUCCESS;
}


void EvrythngDestroyHandle(evrythng_handle_t handle)
{
    if (!handle) return;
    if (handle->initialized && MQTTisConnected(&handle->mqtt_client)) EvrythngDisconnect(handle);

    if (handle->initialized)
    {
        handle->mqtt_thread_stop = 1;
        platform_thread_join(&handle->mqtt_thread, 0x00FFFFFF);
        platform_thread_destroy(&handle->mqtt_thread);
    }

    if (handle->host) platform_free(handle->host);
    if (handle->key) platform_free(handle->key);
    if (handle->client_id) platform_free(handle->client_id);

    sub_callback_t **_sub_callback = &handle->sub_callbacks;
    while (*_sub_callback) 
    {
        sub_callback_t* _sub_callback_tmp = *_sub_callback;
        _sub_callback = &(*_sub_callback)->next;
        platform_free(_sub_callback_tmp->topic);
        platform_free(_sub_callback_tmp);
    }

    MQTTClientDeinit(&handle->mqtt_client);

    platform_mutex_deinit(&handle->next_op_mtx);
    platform_mutex_deinit(&handle->async_op_mtx);
    platform_semaphore_deinit(&handle->next_op_ready_sem);
    platform_semaphore_deinit(&handle->next_op_result_sem);

    platform_free(handle);
}


static int replace_str(char** dest, const char* src, size_t size)
{
    if (*dest) 
        platform_free(*dest);

    *dest = (char*)platform_malloc(size+1); 
    if (!*dest) 
        return EVRYTHNG_MEMORY_ERROR;
    memset(*dest, 0, size+1);

    strncpy(*dest, src, size);

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t EvrythngSetUrl(evrythng_handle_t handle, const char* url)
{
    if (!handle || !url)
        return EVRYTHNG_BAD_ARGS;

    if (strncmp("tcp", url, strlen("tcp")) == 0) 
    {
        debug("setting TCP connection %s", url);
        handle->secure_connection = 0;
    }
    else if (strncmp("ssl", url, strlen("ssl")) == 0) 
    {
        debug("setting SSL connection %s", url);
        handle->secure_connection = 1;
    }
    else return EVRYTHNG_BAD_URL;

    char* delim = strrchr(url, ':');
    if (!delim)
    {
        error("url does not contain port");
        return EVRYTHNG_BAD_URL;
    }

    int port = strtol(delim+1, 0, 10);
    if (port <= 0 || port >= 65536)
    {
        error("url does not contain valid port number");
        return EVRYTHNG_BAD_URL;
    }
    handle->port = port;

    const char* host_ptr = url + strlen("tcp://");

    return replace_str(&handle->host, host_ptr, delim - host_ptr);
}


evrythng_return_t EvrythngSetKey(evrythng_handle_t handle, const char* key)
{
    if (!handle || !key)
        return EVRYTHNG_BAD_ARGS;
    int r = replace_str(&handle->key, key, strlen(key));
    handle->mqtt_conn_opts.password.cstring = handle->key;
    return r;
}


evrythng_return_t EvrythngSetClientId(evrythng_handle_t handle, const char* client_id)
{
    if (!handle || !client_id)
        return EVRYTHNG_BAD_ARGS;
    int r = replace_str(&handle->client_id, client_id, strlen(client_id));
    handle->mqtt_conn_opts.clientID.cstring = handle->client_id;
    return r;
}


evrythng_return_t EvrythngSetLogCallback(evrythng_handle_t handle, evrythng_log_callback callback)
{
    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    handle->log_callback = callback;

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t EvrythngSetConnectionCallbacks(evrythng_handle_t handle, evrythng_callback on_connection_lost,  evrythng_callback on_connection_restored)
{
    if (!handle) return EVRYTHNG_BAD_ARGS;

    handle->on_connection_lost = on_connection_lost;
    handle->on_connection_restored = on_connection_restored;

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t EvrythngSetQos(evrythng_handle_t handle, int qos)
{
    if (!handle || qos < 0 || qos > 2)
        return EVRYTHNG_BAD_ARGS;

    handle->qos = qos;

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t EvrythngSetThreadPriority(evrythng_handle_t handle, int priority)
{
    if (!handle || priority < 0)
        return EVRYTHNG_BAD_ARGS;

    handle->mqtt_thread_priority = priority;

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t EvrythngSetThreadStacksize(evrythng_handle_t handle, int stacksize)
{
    if (!handle || stacksize < 1024)
        return EVRYTHNG_BAD_ARGS;

    handle->mqtt_thread_stacksize = stacksize;

    return EVRYTHNG_SUCCESS;
}


static evrythng_return_t add_sub_callback(evrythng_handle_t handle, const char* topic, int qos, sub_callback *callback)
{
    evrythng_return_t ret = EVRYTHNG_SUCCESS;

    char* qp_start = strstr(topic, "?pubStates=");
    int topic_len = 
        qp_start == NULL ? strlen(topic) : qp_start - topic;

    sub_callback_t **_sub_callbacks = &handle->sub_callbacks;
    while (*_sub_callbacks) 
    {
        qp_start = strstr((*_sub_callbacks)->topic, "?pubStates=");
        int saved_topic_len = 
            qp_start == NULL ? strlen((*_sub_callbacks)->topic) : qp_start - (*_sub_callbacks)->topic;

        if ((topic_len == saved_topic_len) && 
                (strncmp((*_sub_callbacks)->topic, topic, saved_topic_len)) == 0) 
        {
            debug("callback for %s already exists", topic);
            ret = EVRYTHNG_ALREADY_SUBSCRIBED;
            goto out;
        }
        _sub_callbacks = &(*_sub_callbacks)->next;
    }

    if ((*_sub_callbacks = (sub_callback_t*)platform_malloc(sizeof(sub_callback_t))) == NULL) 
    {
        ret = EVRYTHNG_MEMORY_ERROR;
        goto out;
    }

    if (((*_sub_callbacks)->topic = (char*)platform_malloc(strlen(topic) + 1)) == NULL) 
    {
        platform_free(*_sub_callbacks);
        ret = EVRYTHNG_MEMORY_ERROR;
        goto out;
    }

    strcpy((*_sub_callbacks)->topic, topic);
    (*_sub_callbacks)->qos = qos;
    (*_sub_callbacks)->callback = callback;
    (*_sub_callbacks)->next = 0;

out:
    return ret;
}


static evrythng_return_t rm_sub_callback(evrythng_handle_t handle, const char* topic, char* deleted_topic)
{
    evrythng_return_t ret = EVRYTHNG_NOT_SUBSCRIBED;

    sub_callback_t *currP, *prevP = NULL;

    for (currP = handle->sub_callbacks; currP != NULL; prevP = currP, currP = currP->next) 
    {
        int len;
        char* qp = strstr(currP->topic, "?pubStates=");
        if (qp)
            len = qp - currP->topic;
        else
            len = strlen(currP->topic);

        if (strncmp(currP->topic, topic, len) == 0) 
        {
            if (prevP == NULL) 
            {
                /* Fix beginning pointer. */
                handle->sub_callbacks = currP->next;
            } 
            else 
            {
                /*
                 * Fix previous node's next to
                 * skip over the removed node.
                 */
                prevP->next = currP->next;
            }

            if (deleted_topic)
                strcpy(deleted_topic, currP->topic);
            
            /* Deallocate the node. */
            platform_free(currP->topic);
            platform_free(currP);

            /* Done searching. */
            ret = EVRYTHNG_SUCCESS;
            break;
        }
    }

    return ret;
}


static sub_callback* get_sub_callback(evrythng_handle_t handle, MQTTString* topic)
{
    sub_callback* cb = 0;

    sub_callback_t *_sub_callback = handle->sub_callbacks;
    while (_sub_callback) 
    {
        if (MQTTPacket_equals(topic, _sub_callback->topic) || MQTTisTopicMatched(_sub_callback->topic, topic))
        {
            cb = _sub_callback->callback;
            break;
        }
        _sub_callback = _sub_callback->next;
    }

    return cb;
}


void message_callback(MessageData* data, void* userdata)
{
    evrythng_handle_t handle = (evrythng_handle_t)userdata;

    //debug("received msg topic: %s", data->topicName->lenstring.data);
    //debug("received msg, length %d, payload: %s", data->message->payloadlen, (char*)data->message->payload);

    if (data->message->payloadlen < 3) 
    {
        error("incorrect message lenth %d", data->message->payloadlen);
        return;
    }

    sub_callback* cb = get_sub_callback(handle, data->topicName);
    if (cb)
    {
        (*cb)(data->message->payload, data->message->payloadlen);
    }
}


static evrythng_return_t evrythng_async_op(evrythng_handle_t handle, int op, const char* topic, MQTTMessage* message, sub_callback *callback)
{
    evrythng_return_t rc;

    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    platform_mutex_lock(&handle->async_op_mtx);

    platform_mutex_lock(&handle->next_op_mtx);

    handle->next_op.op = op;
    handle->next_op.topic = topic;
    handle->next_op.message = message;
    handle->next_op.callback = callback;

    platform_mutex_unlock(&handle->next_op_mtx);

    platform_semaphore_post(&handle->next_op_ready_sem);

    if (platform_semaphore_wait(&handle->next_op_result_sem, handle->command_timeout_ms * 2))
    {
        platform_mutex_lock(&handle->next_op_mtx);
        handle->next_op.op = MQTT_NOP;
        platform_semaphore_wait(&handle->next_op_result_sem, 0);
        platform_mutex_unlock(&handle->next_op_mtx);
        rc = EVRYTHNG_TIMEOUT;
    }
    else
    {
        rc = handle->next_op.result;
    }

    platform_mutex_unlock(&handle->async_op_mtx);

    return rc;
}

#define MQTT_CLIENTID_LEN 23
static const char* clientid_charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

evrythng_return_t EvrythngConnect(evrythng_handle_t handle)
{
    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    if (!handle->initialized)
    {
        if (!handle->client_id)
        {
            int i;
            handle->client_id = (char*)platform_malloc(MQTT_CLIENTID_LEN+1);
            if (!handle->client_id)
                return EVRYTHNG_MEMORY_ERROR;
            memset(handle->client_id, 0, MQTT_CLIENTID_LEN+1);

            for (i = 0; i < MQTT_CLIENTID_LEN; i++)
                handle->client_id[i] = clientid_charset[platform_rand() % strlen(clientid_charset)];

            handle->mqtt_conn_opts.clientID.cstring = handle->client_id;
            debug("client ID: %s", handle->client_id);
        }

        platform_thread_create(&handle->mqtt_thread, handle->mqtt_thread_priority, "mqtt_thread", mqtt_thread, handle->mqtt_thread_stacksize, (void*)handle);

        handle->initialized = 1;
    }

    if (MQTTisConnected(&handle->mqtt_client))
    {
        warning("already connected");
        return EVRYTHNG_SUCCESS;
    }

    return evrythng_async_op(handle, MQTT_CONNECT, 0, 0, 0);
}

typedef enum _mqtt_connection_status_t {
    MQTT_CONNECTION_ACCEPTED = 0x00,
    MQTT_UNACCEPTABLE_PROTOCOL_VERSION = 0x01,
    MQTT_IDENTIFIER_REJECTED = 0x02,
    MQTT_SERVER_UNAVAILABLE = 0x03,
    MQTT_BAD_USER_NAME_OR_PASSWORD = 0x04,
    MQTT_NOT_AUTHORIZED = 0x05,
} mqtt_connection_status_t;

evrythng_return_t evrythng_connect_internal(evrythng_handle_t handle)
{
    int rc;

    if (MQTTisConnected(&handle->mqtt_client))
    {
        warning("already connected");
        return EVRYTHNG_SUCCESS;
    }

    if (handle->secure_connection)
        platform_network_securedinit(&handle->mqtt_network, handle->ca_buf, handle->ca_size);
    else
        platform_network_init(&handle->mqtt_network);

    int attempt;
    for (attempt = 1; attempt <= 3; attempt++)
    {
        debug("connecting to host: %s, port: %d (%d)", handle->host, handle->port, attempt);
        if (platform_network_connect(&handle->mqtt_network, handle->host, handle->port))
        {
            error("Failed to establish network connection");
            platform_network_disconnect(&handle->mqtt_network);
            continue;
        }
        debug("network connection established");

        if ((rc = MQTTConnect(&handle->mqtt_client, &handle->mqtt_conn_opts)) != MQTT_SUCCESS)
        {
            error("mqtt connection failed, code %d", rc);
            platform_network_disconnect(&handle->mqtt_network);

            switch (rc) {
                case MQTT_NOT_AUTHORIZED:
                    return EVRYTHNG_AUTH_FAILED;

                case MQTT_IDENTIFIER_REJECTED:
                    return EVRYTHNG_CLIENT_ID_REJECTED;

                default:
                    break;
            }
            continue;
        }

        debug("MQTT connected");
        break;
    }

    if (!MQTTisConnected(&handle->mqtt_client))
    {
        return EVRYTHNG_CONNECTION_FAILED;
    }


    sub_callback_t* _sub_callback = handle->sub_callbacks;
    while (_sub_callback) 
    {
        int rc = MQTTSubscribe(
                &handle->mqtt_client, 
                _sub_callback->topic, 
                _sub_callback->qos);
        if (rc >= 0) 
        {
            debug("successfully subscribed to %s", _sub_callback->topic);
        }
        else 
        {
            error("subscription failed, rc = %d", rc);
            break;
        }
        _sub_callback = _sub_callback->next;
    }

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t EvrythngDisconnect(evrythng_handle_t handle)
{
    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    if (!handle->initialized)
        return EVRYTHNG_SUCCESS;

    if (!MQTTisConnected(&handle->mqtt_client))
        return EVRYTHNG_SUCCESS;

    return evrythng_async_op(handle, MQTT_DISCONNECT, 0, 0, 0);
}


evrythng_return_t evrythng_disconnect_internal(evrythng_handle_t handle, int gracefull)
{
    int rc;

    if (!MQTTisConnected(&handle->mqtt_client))
        return EVRYTHNG_SUCCESS;

    if (gracefull)
    {
        sub_callback_t* _sub_callback = handle->sub_callbacks;
        while (_sub_callback) 
        {
            rc = MQTTUnsubscribe(&handle->mqtt_client, _sub_callback->topic);
            if (rc >= 0) 
            {
                debug("successfully unsubscribed from %s", _sub_callback->topic);
            }
            else 
            {
                warning("unsubscription failed, rc = %d", rc);
                break;
            }
            _sub_callback = _sub_callback->next;
        }

        rc = MQTTDisconnect(&handle->mqtt_client);
        if (rc != MQTT_SUCCESS)
        {
            error("failed to disconnect mqtt: rc = %d", rc);
        }
    }
    else
    {
        handle->mqtt_client.isconnected = 0;
    }

    platform_network_disconnect(&handle->mqtt_network);

    debug("MQTT disconnected");

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t evrythng_publish(
        evrythng_handle_t handle, 
        const char* entity, 
        const char* entity_id, 
        const char* data_type, 
        const char* data_name, 
        const char* property_json)
{
    if (!handle) return EVRYTHNG_BAD_ARGS;

    if (!MQTTisConnected(&handle->mqtt_client)) 
    {
        error("%s: client is not connected", __func__);
        return EVRYTHNG_NOT_CONNECTED;
    }

    int rc;
    char pub_topic[TOPIC_MAX_LEN];

    if (entity_id == NULL) 
    {
        rc = snprintf(pub_topic, TOPIC_MAX_LEN, "%s/%s", entity, data_name);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            error("Topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else if (data_name == NULL) 
    {
        rc = snprintf(pub_topic, TOPIC_MAX_LEN, "%s/%s/%s", entity, entity_id, data_type);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            error("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else 
    {
        rc = snprintf(pub_topic, TOPIC_MAX_LEN, "%s/%s/%s/%s", entity, entity_id, data_type, data_name);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            error("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    }

    debug("publish topic: %s", pub_topic);

    MQTTMessage msg = {
        .qos = handle->qos, 
        .retained = 1, 
        .dup = 0,
        .id = 0,
        .payload = (void*)property_json,
        .payloadlen = strlen(property_json)
    };

    return evrythng_async_op(handle, MQTT_PUBLISH, pub_topic, &msg, 0);
}


evrythng_return_t evrythng_subscribe(
        evrythng_handle_t handle, 
        const char* entity, 
        const char* entity_id, 
        const char* data_type, 
        const char* data_name, 
        int pub_states,
        sub_callback *callback)
{
    if (!MQTTisConnected(&handle->mqtt_client)) 
    {
        error("%s: client is not connected", __func__);
        return EVRYTHNG_NOT_CONNECTED;
    }

    int rc;
    char sub_topic[TOPIC_MAX_LEN] = { 0 };

    if (entity_id == NULL) 
    {
        rc = snprintf(sub_topic, TOPIC_MAX_LEN, "%s/%s?pubStates=%d", entity, data_name, pub_states);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else if (data_name == NULL) 
    {
        rc = snprintf(sub_topic, TOPIC_MAX_LEN, "%s/%s/%s?pubStates=%d", entity, entity_id, data_type, pub_states);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else 
    {
        rc = snprintf(sub_topic, TOPIC_MAX_LEN, "%s/%s/%s/%s?pubStates=%d", entity, entity_id, data_type, data_name, pub_states);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    }

    return evrythng_async_op(handle, MQTT_SUBSCRIBE, sub_topic, 0, callback);
}


evrythng_return_t evrythng_unsubscribe(
        evrythng_handle_t handle, 
        const char* entity, 
        const char* entity_id, 
        const char* data_type, 
        const char* data_name)
{
    if (!MQTTisConnected(&handle->mqtt_client)) 
    {
        error("%s: client is not connected", __func__);
        return EVRYTHNG_NOT_CONNECTED;
    }

    int rc;
    char unsub_topic[TOPIC_MAX_LEN];

    if (entity_id == NULL) 
    {
        rc = snprintf(unsub_topic, TOPIC_MAX_LEN, "%s/%s", entity, data_name);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else if (data_name == NULL) 
    {
        rc = snprintf(unsub_topic, TOPIC_MAX_LEN, "%s/%s/%s", entity, entity_id, data_type);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else 
    {
        rc = snprintf(unsub_topic, TOPIC_MAX_LEN, "%s/%s/%s/%s", entity, entity_id, data_type, data_name);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    }

    return evrythng_async_op(handle, MQTT_UNSUBSCRIBE, unsub_topic, 0, 0);
}


static void mqtt_thread(void* arg)
{
    char actual_topic[TOPIC_MAX_LEN];
    int rc = MQTT_SUCCESS;

    evrythng_handle_t handle = (evrythng_handle_t)arg;

    while (!handle->mqtt_thread_stop)
    {
        if (rc == MQTT_CONNECTION_LOST)
        {
            warning("mqtt server connection lost");
            evrythng_disconnect_internal(handle, 0);

            if (handle->on_connection_lost)
                (*handle->on_connection_lost)();

            while (!handle->mqtt_thread_stop)
            {
                if (evrythng_connect_internal(handle) != EVRYTHNG_SUCCESS)
                {
                    platform_printf("could not connect, retrying\n");
                    platform_sleep(300);
                    continue;
                }
                
                if (handle->on_connection_restored)
                    (*handle->on_connection_restored)();
                break;
            }
        }

        if (platform_semaphore_wait(&handle->next_op_ready_sem, 0))
        {
            rc = MQTTYield(&handle->mqtt_client, 300);
            platform_sleep(100);
            continue;
        }

        platform_mutex_lock(&handle->next_op_mtx);

        switch (handle->next_op.op)
        {
            case MQTT_CONNECT:
                handle->next_op.result = evrythng_connect_internal(handle);
                break;

            case MQTT_DISCONNECT:
                handle->next_op.result = evrythng_disconnect_internal(handle, 1);
                break;

            case MQTT_PUBLISH:
                rc = MQTTPublish(&handle->mqtt_client, 
                        handle->next_op.topic,
                        handle->next_op.message);
                if (rc == MQTT_SUCCESS) 
                {
                    debug("published message: %s", handle->next_op.message->payload);
                    handle->next_op.result = EVRYTHNG_SUCCESS;
                }
                else 
                {
                    error("could not publish message, rc = %d", rc);
                    handle->next_op.result = EVRYTHNG_PUBLISH_ERROR;
                }
                break;

            case MQTT_SUBSCRIBE:
                rc = add_sub_callback(handle, handle->next_op.topic, 
                        handle->qos, handle->next_op.callback);

                if (rc != EVRYTHNG_SUCCESS)
                {
                    error("could not add sub topic: %d", rc);
                    handle->next_op.result = rc;
                }
                else
                {
                    rc = MQTTSubscribe(&handle->mqtt_client, 
                            handle->next_op.topic, 
                            handle->qos);
                    if (rc >= 0) 
                    {
                        debug("successfully subscribed to %s, return code %d", handle->next_op.topic, rc);
                        handle->next_op.result = EVRYTHNG_SUCCESS;
                    }
                    else
                    {
                        debug("subscription failed: %d", rc);
                        handle->next_op.result = EVRYTHNG_SUBSCRIPTION_ERROR;
                        rm_sub_callback(handle, handle->next_op.topic, 0);
                    }
                }
                break;

            case MQTT_UNSUBSCRIBE:
                rc = rm_sub_callback(handle, handle->next_op.topic, actual_topic);
                if (rc != EVRYTHNG_SUCCESS)
                {
                    debug("could not remove callback for topic: %s", handle->next_op.topic);
                    handle->next_op.result = rc;
                }
                else
                {
                    rc = MQTTUnsubscribe(&handle->mqtt_client, actual_topic);
                    if (rc >= 0) 
                    {
                        debug("successfully unsubscribed from %s", actual_topic);
                        handle->next_op.result = EVRYTHNG_SUCCESS;
                    }
                    else
                    {
                        handle->next_op.result = EVRYTHNG_UNSUBSCRIPTION_ERROR;
                    }
                }
                break;

            default:
                handle->next_op.result = EVRYTHNG_BAD_ARGS;
                break;
        }

        platform_semaphore_post(&handle->next_op_result_sem);

        platform_mutex_unlock(&handle->next_op_mtx);
    }

    evrythng_disconnect_internal(handle, 1);
}


