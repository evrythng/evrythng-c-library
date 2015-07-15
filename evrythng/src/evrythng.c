#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
#include "evrythng.h"
#include "platform.h"


#define TOPIC_MAX_LEN 128
#define USERNAME "authorization"

static evrythng_return_t evrythng_connect_internal(evrythng_handle_t handle);


typedef struct sub_callback_t {
    char*                   topic;
    int                     qos;
    sub_callback*           callback;
    struct sub_callback_t*  next;
} sub_callback_t;


typedef struct pub_callback_t {
    MQTTClient_deliveryToken  dt;
    pub_callback*             callback;
    struct pub_callback_t*    next;
} pub_callback_t;


struct evrythng_ctx_t {
    char*   url;
    char*   client_id;
    char*   key;
    char*   ca_buf;
    size_t  ca_size;
    int     enable_ssl;
    int     qos;
    int     initialized;

    evrythng_log_callback log_callback;
    connection_lost_callback conlost_callback;

    MQTTClient                mqtt_client;
    MQTTClient_connectOptions mqtt_conn_opts;
    MQTTClient_SSLOptions     mqtt_ssl_opts;

    sub_callback_t *sub_callbacks;
    pub_callback_t *pub_callbacks;
};


static void evrythng_log(evrythng_handle_t handle, evrythng_log_level_t level, const char* fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    if (handle && handle->log_callback)
        handle->log_callback(level, fmt, vl);
    else
    va_end(vl);
}
#define debug(fmt, ...) evrythng_log(handle, EVRYTHNG_LOG_DEBUG, fmt,  ##__VA_ARGS__);
#define warning(fmt, ...) evrythng_log(handle, EVRYTHNG_LOG_WARNING, fmt,  ##__VA_ARGS__);
#define error(fmt, ...) evrythng_log(handle, EVRYTHNG_LOG_ERROR, fmt,  ##__VA_ARGS__);


evrythng_return_t evrythng_init_handle(evrythng_handle_t* handle)
{
    if (!handle) 
        return EVRYTHNG_BAD_ARGS;

    *handle = (evrythng_handle_t)malloc(sizeof(struct evrythng_ctx_t));

    if (!*handle) 
        return EVRYTHNG_MEMORY_ERROR;

    memset(*handle, 0, sizeof(struct evrythng_ctx_t));

    memcpy(&(*handle)->mqtt_conn_opts, &(MQTTClient_connectOptions)MQTTClient_connectOptions_initializer, sizeof(MQTTClient_connectOptions));
    memcpy(&(*handle)->mqtt_ssl_opts, &(MQTTClient_SSLOptions)MQTTClient_SSLOptions_initializer, sizeof(MQTTClient_SSLOptions));

    (*handle)->mqtt_conn_opts.keepAliveInterval = 10;
    (*handle)->mqtt_conn_opts.reliable = 0;
    (*handle)->mqtt_conn_opts.cleansession = 1;
    (*handle)->mqtt_conn_opts.struct_version = 1;
    (*handle)->mqtt_conn_opts.username = USERNAME;
    (*handle)->mqtt_conn_opts.connectTimeout = 60;
    (*handle)->qos = 1;
    (*handle)->log_callback = 0;
    (*handle)->conlost_callback = 0;

    return EVRYTHNG_SUCCESS;
}


void evrythng_destroy_handle(evrythng_handle_t handle)
{
    if (!handle) return;
    if (handle->initialized) evrythng_disconnect(handle);
    if (handle->initialized && handle->mqtt_client) MQTTClient_destroy(&handle->mqtt_client);
    if (handle->url) free(handle->url);
    if (handle->key) free(handle->key);
    if (handle->ca_buf) free(handle->ca_buf);
    if (handle->client_id) free(handle->client_id);

    pub_callback_t **_pub_callback = &handle->pub_callbacks;
    while (*_pub_callback) 
    {
        pub_callback_t* _pub_callback_tmp = *_pub_callback;
        _pub_callback = &(*_pub_callback)->next;
        free(_pub_callback_tmp);
    }

    sub_callback_t **_sub_callback = &handle->sub_callbacks;
    while(*_sub_callback) 
    {
        sub_callback_t* _sub_callback_tmp = *_sub_callback;
        _sub_callback = &(*_sub_callback)->next;
        free(_sub_callback_tmp->topic);
        free(_sub_callback_tmp);
    }

    free(handle);
}


static int replace_str(char** dest, const char* src, size_t size)
{
    if (*dest) 
        free(*dest);

    *dest = (char*)malloc(size); 
    if (!*dest) 
        return EVRYTHNG_MEMORY_ERROR;

    strncpy(*dest, src, size);

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t evrythng_set_url(evrythng_handle_t handle, const char* url)
{
    if (!handle || !url)
        return EVRYTHNG_BAD_ARGS;

    if (strncmp("tcp", url, strlen("tcp")) == 0) 
    {
        debug("setting TCP connection %s", url);
        handle->enable_ssl = 0;
    }
    else if (strncmp("ssl", url, strlen("ssl")) == 0) 
    {
        debug("setting SSL connection %s", url);
        handle->enable_ssl = 1;
        handle->mqtt_ssl_opts.enableServerCertAuth = 0;
        handle->mqtt_conn_opts.struct_version = 4;
        handle->mqtt_conn_opts.ssl = &handle->mqtt_ssl_opts;
    }
    else return EVRYTHNG_BAD_URL;

    return replace_str(&handle->url, url, strlen(url) + 1);
}


evrythng_return_t evrythng_set_key(evrythng_handle_t handle, const char* key)
{
    if (!handle || !key)
        return EVRYTHNG_BAD_ARGS;
    int r = replace_str(&handle->key, key, strlen(key) + 1);
    handle->mqtt_conn_opts.password = handle->key;
    return r;
}


evrythng_return_t evrythng_set_client_id(evrythng_handle_t handle, const char* client_id)
{
    if (!handle || !client_id)
        return EVRYTHNG_BAD_ARGS;
    return replace_str(&handle->client_id, client_id, strlen(client_id) + 1);
}


evrythng_return_t evrythng_set_certificate(evrythng_handle_t handle, const char* cert, size_t size)
{
    if (!handle || !cert || size <= 0)
        return EVRYTHNG_BAD_ARGS;
    int r = replace_str(&handle->ca_buf, cert, size);
    handle->ca_size = size;
    handle->mqtt_ssl_opts.trustStore = handle->ca_buf;
#if defined(NO_FILESYSTEM)
    handle->mqtt_ssl_opts.trustStore_size = size;
#endif
    return r;
}


evrythng_return_t evrythng_set_log_callback(evrythng_handle_t handle, evrythng_log_callback callback)
{
    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    handle->log_callback = callback;

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t evrythng_set_conlost_callback(evrythng_handle_t handle, connection_lost_callback callback)
{
    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    handle->conlost_callback = callback;

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t evrythng_set_qos(evrythng_handle_t handle, int qos)
{
    if (!handle || qos < 0 || qos > 2)
        return EVRYTHNG_BAD_ARGS;

    handle->qos = qos;

    return EVRYTHNG_SUCCESS;
}


static evrythng_return_t add_sub_callback(evrythng_handle_t handle, char* topic, int qos, sub_callback *callback)
{
    sub_callback_t **_sub_callbacks = &handle->sub_callbacks;
    while (*_sub_callbacks) 
    {
        _sub_callbacks = &(*_sub_callbacks)->next;
    }

    if ((*_sub_callbacks = (sub_callback_t*)malloc(sizeof(sub_callback_t))) == NULL) 
    {
        return EVRYTHNG_MEMORY_ERROR;
    }

    if (((*_sub_callbacks)->topic = (char*)malloc(strlen(topic) + 1)) == NULL) 
    {
        free(*_sub_callbacks);
        return EVRYTHNG_MEMORY_ERROR;
    }

    strcpy((*_sub_callbacks)->topic, topic);
    (*_sub_callbacks)->qos = qos;
    (*_sub_callbacks)->callback = callback;
    (*_sub_callbacks)->next = 0;

    return EVRYTHNG_SUCCESS;
}


static void rm_sub_callback(evrythng_handle_t handle, const char* topic)
{
    sub_callback_t *_sub_callback = handle->sub_callbacks;

    if (!_sub_callback) 
        return;

    if (strcmp(_sub_callback->topic, topic) == 0) 
    {
        handle->sub_callbacks = _sub_callback->next;
        free(_sub_callback->topic);
        free(_sub_callback);
        return;
    }

    while (_sub_callback->next) 
    {
        if (strcmp(_sub_callback->next->topic, topic) == 0) 
        {
            sub_callback_t* _sub_callback_tmp = _sub_callback->next;

            _sub_callback->next = _sub_callback->next->next;

            free(_sub_callback_tmp->topic);
            free(_sub_callback_tmp);
            continue;
        }
        _sub_callback = _sub_callback->next;
    }
}


static evrythng_return_t add_pub_callback(evrythng_handle_t handle, MQTTClient_deliveryToken dt, pub_callback *callback)
{
    pub_callback_t **_pub_callbacks = &handle->pub_callbacks;
    while(*_pub_callbacks) 
    {
        if ((*_pub_callbacks)->callback == NULL) 
        {
            (*_pub_callbacks)->dt = dt;
            (*_pub_callbacks)->callback = callback;
            return EVRYTHNG_SUCCESS;
        }
        _pub_callbacks = &(*_pub_callbacks)->next;
    }

    if ((*_pub_callbacks = (pub_callback_t*)malloc(sizeof(pub_callback_t))) == NULL) 
    {
        return EVRYTHNG_MEMORY_ERROR;
    }

    (*_pub_callbacks)->dt = dt;
    (*_pub_callbacks)->callback = callback;
    (*_pub_callbacks)->next = 0;

    return EVRYTHNG_SUCCESS;
}


void connection_lost_callback_internal(void* context, char* cause)
{
    evrythng_handle_t handle = (evrythng_handle_t)context;

    warning("connection lost");

    pub_callback_t **_pub_callbacks = &handle->pub_callbacks;
    while(*_pub_callbacks) 
    {
        (*_pub_callbacks)->callback = NULL;
        _pub_callbacks = &(*_pub_callbacks)->next;
    }

    if (handle->conlost_callback)
        (*handle->conlost_callback)(handle);
}


int message_callback(void* context, char* topic_name, int topic_len, MQTTClient_message* message)
{
    evrythng_handle_t handle = (evrythng_handle_t)context;

    debug("received msg topic: %s", topic_name);
    debug("received msg, length %d, payload: %s", message->payloadlen, (char*)message->payload);

    if (message->payloadlen < 3) 
    {
        error("incorrect message lenth %d", message->payloadlen);
        goto exit;
    }

    sub_callback_t **_sub_callbacks = &handle->sub_callbacks;
    while (*_sub_callbacks) 
    {
        if (strcmp((*_sub_callbacks)->topic, topic_name) == 0) 
        {
            (*((*_sub_callbacks)->callback))((const char*)message->payload, message->payloadlen);
        }
        _sub_callbacks = &(*_sub_callbacks)->next;
    }

exit:
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic_name);

    return 1;
}


void delivery_complete_callback(void* context, MQTTClient_deliveryToken dt)
{
    evrythng_handle_t handle = (evrythng_handle_t)context;

    debug("%s: delivery complete, dt = %d", __func__, dt);

    pub_callback_t **_pub_callbacks = &handle->pub_callbacks;
    while(*_pub_callbacks) 
    {
        if ((*_pub_callbacks)->dt == dt) 
        {
            (*((*_pub_callbacks)->callback))();
            (*_pub_callbacks)->callback = NULL;
            return;
        }
        _pub_callbacks = &(*_pub_callbacks)->next;
    }
}


evrythng_return_t evrythng_connect(evrythng_handle_t handle)
{
    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    if (handle->initialized)
        return evrythng_connect_internal(handle);

    if (handle->enable_ssl && !handle->mqtt_ssl_opts.trustStore) 
    {
        warning("a certificate is required to connect to %s", handle->url);
        return EVRYTHNG_CERT_REQUIRED_ERROR;
    }

    if (!handle->client_id)
    {
        int i;
        handle->client_id = (char*)malloc(10);
        if (!handle->client_id)
            return EVRYTHNG_MEMORY_ERROR;
        memset(handle->client_id, 0, 10);

        for (i = 0; i < 9; i++)
            handle->client_id[i] = '0' + rand() % 10;
        debug("client ID: %s", handle->client_id);
    }

    MQTTClient_init();

    int rc = MQTTClient_create(&handle->mqtt_client, handle->url, handle->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        error("mqtt client creation error: %d", rc);
        return EVRYTHNG_FAILURE;
    }

    if (MQTTClient_setCallbacks(
                handle->mqtt_client, 
                (void*)handle, 
                connection_lost_callback_internal, 
                message_callback, 
                delivery_complete_callback) != MQTTCLIENT_SUCCESS)
    {
        return EVRYTHNG_FAILURE;
    }

    handle->initialized = 1;

    return evrythng_connect_internal(handle);
}


evrythng_return_t evrythng_connect_internal(evrythng_handle_t handle)
{
    int rc;

    if (MQTTClient_isConnected(handle->mqtt_client)) 
        return EVRYTHNG_SUCCESS;

    debug("MQTT connecting to %s", handle->url);
    if ((rc = MQTTClient_connect(handle->mqtt_client, &handle->mqtt_conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        error("Failed to connect, return code %d", rc);
        return EVRYTHNG_CONNECTION_FAILED;
    }
    debug("MQTT connected");

    sub_callback_t **_sub_callbacks = &handle->sub_callbacks;
    while (*_sub_callbacks) 
    {
        int rc = MQTTClient_subscribe(handle->mqtt_client, (*_sub_callbacks)->topic, (*_sub_callbacks)->qos);
        if (rc == MQTTCLIENT_SUCCESS) 
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


evrythng_return_t evrythng_disconnect(evrythng_handle_t handle)
{
    int rc;

    if (!handle)
        return EVRYTHNG_BAD_ARGS;

    if (!handle->initialized)
        return EVRYTHNG_SUCCESS;

    if (!MQTTClient_isConnected(handle->mqtt_client))
        return EVRYTHNG_SUCCESS;

    sub_callback_t **_sub_callbacks = &handle->sub_callbacks;
    while (*_sub_callbacks) 
    {
        rc = MQTTClient_unsubscribe(handle->mqtt_client, (*_sub_callbacks)->topic);
        if (rc == MQTTCLIENT_SUCCESS) 
        {
            debug("successfully unsubscribed from %s", (*_sub_callbacks)->topic);
        }
        else 
        {
            warning("unsubscription failed, rc = %d", rc);
        }
        _sub_callbacks = &(*_sub_callbacks)->next;
    }

    rc = MQTTClient_disconnect(handle->mqtt_client, 1000);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        error("failed to disconnect: rc = %d", rc);
        return EVRYTHNG_FAILURE;
    }
    debug("MQTT disconnected");


    return EVRYTHNG_SUCCESS;
}


static evrythng_return_t evrythng_publish(
        evrythng_handle_t handle, 
        const char* entity, 
        const char* entity_id, 
        const char* data_type, 
        const char* data_name, 
        const char* property_json, 
        pub_callback *callback)
{
    if (!handle) return EVRYTHNG_BAD_ARGS;

    if (!MQTTClient_isConnected(handle->mqtt_client)) 
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

    MQTTClient_deliveryToken dt;
    rc = MQTTClient_publish(handle->mqtt_client, pub_topic, strlen(property_json), (void*)property_json, handle->qos, 0, &dt);
    if (rc == MQTTCLIENT_SUCCESS) 
    {
        debug("published message: %s", property_json);
    }
    else 
    {
        error("could not publish message, rc = %d", rc);
        return EVRYTHNG_PUBLISH_ERROR;
    }

    if (callback != NULL) 
    {
        rc = add_pub_callback(handle, dt, callback);
        if (rc != EVRYTHNG_SUCCESS) 
        {
            error("could not add publish callback");
            return rc;
        }
    }

    return EVRYTHNG_SUCCESS;
}


static evrythng_return_t evrythng_subscribe(
        evrythng_handle_t handle, 
        const char* entity, 
        const char* entity_id, 
        const char* data_type, 
        const char* data_name, 
        sub_callback *callback)
{
    if (!MQTTClient_isConnected(handle->mqtt_client)) 
    {
        error("client is not connected");
        return EVRYTHNG_NOT_CONNECTED;
    }

    int rc;
    char sub_topic[TOPIC_MAX_LEN];

    if (entity_id == NULL) 
    {
        rc = snprintf(sub_topic, TOPIC_MAX_LEN, "%s/%s", entity, data_name);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else if (data_name == NULL) 
    {
        rc = snprintf(sub_topic, TOPIC_MAX_LEN, "%s/%s/%s", entity, entity_id, data_type);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) 
        {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    } 
    else 
    {
        rc = snprintf(sub_topic, TOPIC_MAX_LEN, "%s/%s/%s/%s", entity, entity_id, data_type, data_name);
        if (rc < 0 || rc >= TOPIC_MAX_LEN) {
            debug("topic overflow");
            return EVRYTHNG_BAD_ARGS;
        }
    }

    rc = add_sub_callback(handle, sub_topic, handle->qos, callback);
    if (rc != EVRYTHNG_SUCCESS) 
    {
        error("could not add subscription callback");
        return rc;
    }

    debug("subscribing to topic: %s", sub_topic);

    rc = MQTTClient_subscribe(handle->mqtt_client, sub_topic, handle->qos);
    if (rc == MQTTCLIENT_SUCCESS) 
    {
        debug("successfully subscribed to %s", sub_topic);
    }
    else 
    {
        debug("subscribtion failed, rc=%d", rc);
        return EVRYTHNG_SUBSCRIPTION_ERROR;
    }

    return EVRYTHNG_SUCCESS;
}


static evrythng_return_t evrythng_unsubscribe(
        evrythng_handle_t handle, 
        const char* entity, 
        const char* entity_id, 
        const char* data_type, 
        const char* data_name)
{
    if (!MQTTClient_isConnected(handle->mqtt_client)) 
    {
        error("client is not connected");
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

    rm_sub_callback(handle, unsub_topic);

    rc = MQTTClient_unsubscribe(handle->mqtt_client, unsub_topic);
    if (rc == MQTTCLIENT_SUCCESS) 
    {
        debug("unsubscribed from %s", unsub_topic);
    }
    else 
    {
        error("unsubscription failed, rc=%d", rc);
        return EVRYTHNG_UNSUBSCRIPTION_ERROR;
    }

    return EVRYTHNG_SUCCESS;
}


evrythng_return_t evrythng_publish_thng_property(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name, 
        const char* property_json, 
        pub_callback *callback)
{
    if (!thng_id || !property_name || !property_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "properties", property_name, property_json, callback);
}


evrythng_return_t evrythng_subscribe_thng_property(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name, 
        sub_callback *callback)
{
    if (!thng_id || !property_name || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "properties", property_name, callback);
}

evrythng_return_t evrythng_unsubscribe_thng_property(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name)
{
    if (!thng_id || !property_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "properties", property_name);
}


evrythng_return_t evrythng_subscribe_thng_properties(
        evrythng_handle_t handle, 
        const char* thng_id, 
        sub_callback *callback)
{
    if (!thng_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "properties", NULL, callback);
}


evrythng_return_t evrythng_unsubscribe_thng_properties(
        evrythng_handle_t handle, 
        const char* thng_id)
{
    if (!thng_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "properties", NULL);
}


evrythng_return_t evrythng_publish_thng_properties(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* properties_json, 
        pub_callback *callback)
{
    if (!thng_id || !properties_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "properties", NULL, properties_json, callback);
}


evrythng_return_t evrythng_subscribe_thng_action(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* action_name, 
        sub_callback *callback)
{
    if (!thng_id || !action_name || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "actions", action_name, callback);
}


evrythng_return_t evrythng_unsubscribe_thng_action(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* action_name)
{
    if (!thng_id || !action_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "actions", action_name);
}


evrythng_return_t evrythng_subscribe_thng_actions(
        evrythng_handle_t handle, 
        const char* thng_id, 
        sub_callback *callback)
{
    if (!thng_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "actions", "all", callback);
}


evrythng_return_t evrythng_unsubscribe_thng_actions(
        evrythng_handle_t handle, 
        const char* thng_id)
{
    if (!thng_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "actions", "all");
}


evrythng_return_t evrythng_publish_thng_action(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* action_name, 
        const char* action_json, 
        pub_callback *callback)
{
    if (!thng_id || !action_name || !action_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "actions", action_name, action_json, callback);
}


evrythng_return_t evrythng_publish_thng_actions(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* actions_json, 
        pub_callback *callback)
{
    if (!thng_id || !actions_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "actions", "all", actions_json, callback);
}


evrythng_return_t evrythng_subscribe_thng_location(
        evrythng_handle_t handle, 
        const char* thng_id, 
        sub_callback *callback)
{
    if (!thng_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "location", NULL, callback);
}


evrythng_return_t evrythng_unsubscribe_thng_location(
        evrythng_handle_t handle, 
        const char* thng_id)
{
    if (!thng_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "location", NULL);
}


evrythng_return_t evrythng_publish_thng_location(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* location_json, 
        pub_callback *callback)
{
    if (!thng_id || !location_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "location", NULL, location_json, callback);
}


evrythng_return_t evrythng_subscribe_product_property(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* property_name, 
        sub_callback *callback)
{
    if (!product_id || !property_name || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "products", product_id, "properties", property_name, callback);
}


evrythng_return_t evrythng_unsubscribe_product_property(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* property_name)
{
    if (!product_id || !property_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "products", product_id, "properties", property_name);
}


evrythng_return_t evrythng_subscribe_product_properties(
        evrythng_handle_t handle, 
        const char* product_id, 
        sub_callback *callback)
{
    if (!product_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "products", product_id, "properties", NULL, callback);
}


evrythng_return_t evrythng_unsubscribe_product_properties(
        evrythng_handle_t handle, 
        const char* product_id)
{
    if (!product_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "products", product_id, "properties", NULL);
}


evrythng_return_t evrythng_publish_product_property(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* property_name, 
        const char* property_json, 
        pub_callback *callback)
{
    if (!product_id || !property_name || !property_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "properties", property_name, property_json, callback);
}


evrythng_return_t evrythng_publish_product_properties(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* properties_json, 
        pub_callback *callback)
{
    if (!product_id || !properties_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "properties", NULL, properties_json, callback);
}


evrythng_return_t evrythng_subscribe_product_action(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* action_name, 
        sub_callback *callback)
{
    if (!product_id || !action_name || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "products", product_id, "actions", action_name, callback);
}


evrythng_return_t evrythng_unsubscribe_product_action(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* action_name)
{
    if (!product_id || !action_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "products", product_id, "actions", action_name);
}


evrythng_return_t evrythng_subscribe_product_actions(
        evrythng_handle_t handle, 
        const char* product_id, 
        sub_callback *callback)
{
    if (!product_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "products", product_id, "actions", "all", callback);
}


evrythng_return_t evrythng_unsubscribe_product_actions(
        evrythng_handle_t handle, 
        const char* product_id)
{
    if (!product_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "products", product_id, "actions", "all");
}


evrythng_return_t evrythng_publish_product_action(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* action_name, 
        const char* action_json, 
        pub_callback *callback)
{
    if (!product_id || !action_name || !action_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "actions", action_name, action_json, callback);
}


evrythng_return_t evrythng_publish_product_actions(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* actions_json, 
        pub_callback *callback)
{
    if (!product_id || !actions_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "actions", "all", actions_json, callback);
}


evrythng_return_t evrythng_subscribe_action(
        evrythng_handle_t handle, 
        const char* action_name, 
        sub_callback *callback)
{
    if (!action_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "actions", NULL, NULL, action_name, callback);
}


evrythng_return_t evrythng_unsubscribe_action(
        evrythng_handle_t handle, 
        const char* action_name)
{
    if (!action_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "actions", NULL, NULL, action_name);
}


evrythng_return_t evrythng_subscribe_actions(evrythng_handle_t handle, sub_callback *callback)
{
    if (!callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "actions", NULL, NULL, "all", callback);
}


evrythng_return_t evrythng_unsubscribe_actions(evrythng_handle_t handle)
{
    return evrythng_unsubscribe(handle, "actions", NULL, NULL, "all");
}


evrythng_return_t evrythng_publish_action(
        evrythng_handle_t handle, 
        const char* action_name, 
        const char* action_json, 
        pub_callback *callback)
{
    if (!action_name || !action_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "actions", NULL, NULL, action_name, action_json, callback);
}


evrythng_return_t evrythng_publish_actions(
        evrythng_handle_t handle, 
        const char* actions_json, 
        pub_callback *callback)
{
    if (!actions_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "actions", NULL, NULL, "all", actions_json, callback);
}
