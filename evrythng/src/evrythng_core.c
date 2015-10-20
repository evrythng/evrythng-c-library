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
static evrythng_return_t evrythng_connect_internal(evrythng_handle_t handle);
static evrythng_return_t evrythng_disconnect_internal(evrythng_handle_t handle);

typedef struct sub_callback_t {
    char*                   topic;
    int                     qos;
    sub_callback*           callback;
    struct sub_callback_t*  next;
} sub_callback_t;


enum { MQTT_CONNECT, MQTT_DISCONNECT, MQTT_PUBLISH, MQTT_SUBSCRIBE, MQTT_UNSUBSCRIBE };
typedef struct mqtt_op 
{
    int op;
    const char* topic;
    MQTTMessage* message;
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

    (*handle)->command_timeout_ms = 5000;

	MQTTClientInit(
            &(*handle)->mqtt_client, 
            &(*handle)->mqtt_network, 
            (*handle)->command_timeout_ms, 
            (*handle)->serialize_buffer, sizeof((*handle)->serialize_buffer), 
            (*handle)->read_buffer, sizeof((*handle)->read_buffer));

    (*handle)->mqtt_thread_stacksize = 8192;

    MutexInit(&(*handle)->next_op_mtx);
    SemaphoreInit(&(*handle)->next_op_ready_sem);
    SemaphoreInit(&(*handle)->next_op_result_sem);

    return EVRYTHNG_SUCCESS;
}


void EvrythngDestroyHandle(evrythng_handle_t handle)
{
    if (!handle) return;
    if (handle->initialized && MQTTisConnected(&handle->mqtt_client)) EvrythngDisconnect(handle);

    if (handle->initialized)
    {
        handle->mqtt_thread_stop = 1;
        ThreadJoin(&handle->mqtt_thread, 0x00FFFFFF);
        ThreadDestroy(&handle->mqtt_thread);
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

    MutexDeinit(&handle->next_op_mtx);
    SemaphoreDeinit(&handle->next_op_ready_sem);
    SemaphoreDeinit(&handle->next_op_result_sem);

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


static sub_callback_t* add_sub_callback(evrythng_handle_t handle, char* topic, int qos, sub_callback *callback)
{
    sub_callback_t **_sub_callbacks = &handle->sub_callbacks;
    while (*_sub_callbacks) 
    {
        _sub_callbacks = &(*_sub_callbacks)->next;
    }

    if ((*_sub_callbacks = (sub_callback_t*)platform_malloc(sizeof(sub_callback_t))) == NULL) 
    {
        return 0;
    }

    if (((*_sub_callbacks)->topic = (char*)platform_malloc(strlen(topic) + 1)) == NULL) 
    {
        platform_free(*_sub_callbacks);
        return 0;
    }

    strcpy((*_sub_callbacks)->topic, topic);
    (*_sub_callbacks)->qos = qos;
    (*_sub_callbacks)->callback = callback;
    (*_sub_callbacks)->next = 0;

    return *_sub_callbacks;
}


static void rm_sub_callback(evrythng_handle_t handle, const char* topic)
{
    sub_callback_t *_sub_callback = handle->sub_callbacks;

    if (!_sub_callback) 
        return;

    if (strcmp(_sub_callback->topic, topic) == 0) 
    {
        handle->sub_callbacks = _sub_callback->next;
        platform_free(_sub_callback->topic);
        platform_free(_sub_callback);
        return;
    }

    while (_sub_callback->next) 
    {
        if (strcmp(_sub_callback->next->topic, topic) == 0) 
        {
            sub_callback_t* _sub_callback_tmp = _sub_callback->next;

            _sub_callback->next = _sub_callback->next->next;

            platform_free(_sub_callback_tmp->topic);
            platform_free(_sub_callback_tmp);
            continue;
        }
        _sub_callback = _sub_callback->next;
    }
}



static void message_callback(MessageData* data, void* userdata)
{
    evrythng_handle_t handle = (evrythng_handle_t)userdata;

    //debug("received msg topic: %s", data->topicName->lenstring.data);
    //debug("received msg, length %d, payload: %s", data->message->payloadlen, (char*)data->message->payload);

    if (data->message->payloadlen < 3) 
    {
        error("incorrect message lenth %d", data->message->payloadlen);
        return;
    }

    sub_callback_t *_sub_callback = handle->sub_callbacks;
    while (_sub_callback) 
    {
        if (MQTTPacket_equals(data->topicName, _sub_callback->topic) || MQTTisTopicMatched(_sub_callback->topic, data->topicName))
        {
            (*(_sub_callback->callback))(data->message->payload, data->message->payloadlen);
        }
        _sub_callback = _sub_callback->next;
    }
}


static evrythng_return_t evrythng_async_op(evrythng_handle_t handle, int op, const char* topic, MQTTMessage* message)
{
    evrythng_return_t rc;

    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    MutexLock(&handle->next_op_mtx);

    handle->next_op.op = op;
    handle->next_op.topic = topic;
    handle->next_op.message = message;

    SemaphorePost(&handle->next_op_ready_sem);

    if (SemaphoreWait(&handle->next_op_result_sem, handle->command_timeout_ms * 2))
    {
        rc = EVRYTHNG_TIMEOUT;
    }
    else
    {
        rc = handle->next_op.result;
    }

    MutexUnlock(&handle->next_op_mtx);

    return rc;
}


evrythng_return_t EvrythngConnect(evrythng_handle_t handle)
{
    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    if (!handle->initialized)
    {
        if (!handle->client_id)
        {
            int i;
            handle->client_id = (char*)platform_malloc(10);
            if (!handle->client_id)
                return EVRYTHNG_MEMORY_ERROR;
            memset(handle->client_id, 0, 10);

            for (i = 0; i < 9; i++)
                handle->client_id[i] = '0' + rand() % 10;
            handle->mqtt_conn_opts.clientID.cstring = handle->client_id;
            debug("client ID: %s", handle->client_id);
        }

        ThreadCreate(&handle->mqtt_thread, handle->mqtt_thread_priority, "mqtt_thread", mqtt_thread, handle->mqtt_thread_stacksize, (void*)handle);

        handle->initialized = 1;
    }

    if (MQTTisConnected(&handle->mqtt_client))
    {
        warning("already connected");
        return EVRYTHNG_SUCCESS;
    }

    return evrythng_async_op(handle, MQTT_CONNECT, 0, 0);
}


evrythng_return_t evrythng_connect_internal(evrythng_handle_t handle)
{
    int rc;

    if (MQTTisConnected(&handle->mqtt_client))
    {
        warning("already connected");
        return EVRYTHNG_SUCCESS;
    }

    if (handle->secure_connection)
        NetworkSecuredInit(&handle->mqtt_network, handle->ca_buf, handle->ca_size);
    else
        NetworkInit(&handle->mqtt_network);

    int attempt;
    for (attempt = 1; attempt <= 3; attempt++)
    {
        debug("connecting to host: %s, port: %d (%d)", handle->host, handle->port, attempt);
        if (NetworkConnect(&handle->mqtt_network, handle->host, handle->port))
        {
            error("Failed to establish network connection");
            NetworkDisconnect(&handle->mqtt_network);
            continue;
        }
        debug("network connection established");

        if ((rc = MQTTConnect(&handle->mqtt_client, &handle->mqtt_conn_opts)) != MQTT_SUCCESS)
        {
            error("Failed to connect, return code %d", rc);
            NetworkDisconnect(&handle->mqtt_network);
            continue;
        }
        debug("MQTT connected");
        break;
    }

    if (!MQTTisConnected(&handle->mqtt_client))
    {
        return EVRYTHNG_CONNECTION_FAILED;
    }

    sub_callback_t **_sub_callbacks = &handle->sub_callbacks;
    while (*_sub_callbacks) 
    {
        int rc = MQTTSubscribe(
                &handle->mqtt_client, 
                (*_sub_callbacks)->topic, 
                (*_sub_callbacks)->qos,
                message_callback,
                handle);
        if (rc >= 0) 
        {
            debug("successfully subscribed to %s", (*_sub_callbacks)->topic);
        }
        else 
        {
            error("subscription failed, rc = %d", rc);
        }
        _sub_callbacks = &(*_sub_callbacks)->next;
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

    return evrythng_async_op(handle, MQTT_DISCONNECT, 0, 0);
}


evrythng_return_t evrythng_disconnect_internal(evrythng_handle_t handle)
{
    int rc;

    if (!MQTTisConnected(&handle->mqtt_client))
        return EVRYTHNG_SUCCESS;

    sub_callback_t **_sub_callbacks = &handle->sub_callbacks;
    while (*_sub_callbacks) 
    {
        rc = MQTTUnsubscribe(&handle->mqtt_client, (*_sub_callbacks)->topic);
        if (rc >= 0) 
        {
            debug("successfully unsubscribed from %s", (*_sub_callbacks)->topic);
        }
        else 
        {
            warning("unsubscription failed, rc = %d", rc);
        }
        _sub_callbacks = &(*_sub_callbacks)->next;
    }

    rc = MQTTDisconnect(&handle->mqtt_client);
    if (rc != MQTT_SUCCESS)
    {
        error("failed to disconnect mqtt: rc = %d", rc);
    }
    NetworkDisconnect(&handle->mqtt_network);
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
        error("client is not connected");
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

    return evrythng_async_op(handle, MQTT_PUBLISH, pub_topic, &msg);
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
        error("client is not connected");
        return EVRYTHNG_NOT_CONNECTED;
    }

    int rc;
    char sub_topic[TOPIC_MAX_LEN];

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

    sub_callback_t* new_callback = add_sub_callback(handle, sub_topic, handle->qos, callback);
    if (!new_callback) 
    {
        error("could not add subscription callback");
        return EVRYTHNG_MEMORY_ERROR;
    }

    evrythng_return_t ret = evrythng_async_op(handle, MQTT_SUBSCRIBE, new_callback->topic, 0);
    if (ret != EVRYTHNG_SUCCESS) 
    {
        debug("subscription failed, ret = %d", ret);
        rm_sub_callback(handle, sub_topic);
        return ret;
    }

    return EVRYTHNG_SUCCESS;
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
        error("client is not connected");
        return EVRYTHNG_NOT_CONNECTED;
    }

    int rc;
    char unsub_topic[TOPIC_MAX_LEN];

    if (entity_id == NULL) 
    {
        rc = snprintf(unsub_topic, TOPIC_MAX_LEN, "%s/%s?pubStates=1", entity, data_name);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else if (data_name == NULL) 
    {
        rc = snprintf(unsub_topic, TOPIC_MAX_LEN, "%s/%s/%s?pubStates=1", entity, entity_id, data_type);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else 
    {
        rc = snprintf(unsub_topic, TOPIC_MAX_LEN, "%s/%s/%s/%s?pubStates=1", entity, entity_id, data_type, data_name);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    }

    rm_sub_callback(handle, unsub_topic);

    return evrythng_async_op(handle, MQTT_UNSUBSCRIBE, unsub_topic, 0);
}


static void mqtt_thread(void* arg)
{
    int rc = MQTT_SUCCESS;

    evrythng_handle_t handle = (evrythng_handle_t)arg;

    while (!handle->mqtt_thread_stop)
    {
        if (rc == MQTT_CONNECTION_LOST)
        {
            warning("mqtt server connection lost");
            evrythng_disconnect_internal(handle);

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

        if (SemaphoreWait(&handle->next_op_ready_sem, 0))
        {
            rc = MQTTYield(&handle->mqtt_client, 300);
            platform_sleep(100);
            continue;
        }

        switch (handle->next_op.op)
        {
            case MQTT_CONNECT:
                handle->next_op.result = evrythng_connect_internal(handle);
                break;

            case MQTT_DISCONNECT:
                handle->next_op.result = evrythng_disconnect_internal(handle);
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
                rc = MQTTSubscribe(&handle->mqtt_client, 
                        handle->next_op.topic, 
                        handle->qos, 
                        message_callback, 
                        handle);
                if (rc >= 0) 
                {
                    debug("successfully subscribed to %s", handle->next_op.topic);
                    handle->next_op.result = EVRYTHNG_SUCCESS;
                }
                else
                    handle->next_op.result = EVRYTHNG_SUBSCRIPTION_ERROR;
                break;

            case MQTT_UNSUBSCRIBE:
                rc = MQTTUnsubscribe(&handle->mqtt_client, handle->next_op.topic);
                if (rc >= 0) 
                {
                    debug("successfully unsubscribed from %s", handle->next_op.topic);
                    handle->next_op.result = EVRYTHNG_SUCCESS;
                }
                else
                    handle->next_op.result = EVRYTHNG_UNSUBSCRIPTION_ERROR;
                break;

            default:
                handle->next_op.result = EVRYTHNG_BAD_ARGS;
                break;
        }

        SemaphorePost(&handle->next_op_result_sem);
    }
}


