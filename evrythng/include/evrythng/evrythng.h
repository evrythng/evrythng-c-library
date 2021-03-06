/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 */

/*! \mainpage API documentation
 *
 * \link evrythng.h \endlink
 *
 */

/**
 *
 * @file   evrythng.h
 * @brief  Evrythng API
 *
 **/


#ifndef _EVRYTHNG_H
#define _EVRYTHNG_H


#include <stdlib.h>
#include <stdarg.h>

#define EVRYTHNG_MAJOR_VERSION 1
#define EVRYTHNG_MINOR_VERSION 5

typedef enum _evrythng_return_t 
{
    EVRYTHNG_CLIENT_ID_REJECTED  = -15,
    EVRYTHNG_AUTH_FAILED         = -14,
    EVRYTHNG_NOT_SUBSCRIBED      = -13,
    EVRYTHNG_ALREADY_SUBSCRIBED  = -12,
    EVRYTHNG_TIMEOUT             = -11,
    EVRYTHNG_UNSUBSCRIPTION_ERROR= -10,
    EVRYTHNG_SUBSCRIPTION_ERROR  = -9,
    EVRYTHNG_PUBLISH_ERROR       = -8,
    EVRYTHNG_NOT_CONNECTED       = -7,
    EVRYTHNG_CONNECTION_FAILED   = -6,
    EVRYTHNG_BAD_URL             = -5,
    EVRYTHNG_CERT_REQUIRED_ERROR = -4,
    EVRYTHNG_MEMORY_ERROR        = -3,
    EVRYTHNG_BAD_ARGS            = -2,
    EVRYTHNG_FAILURE             = -1,
    EVRYTHNG_SUCCESS             =  0,
} evrythng_return_t;


typedef enum 
{
    EVRYTHNG_LOG_ERROR   = 0, 
    EVRYTHNG_LOG_WARNING = 1, 
    EVRYTHNG_LOG_DEBUG   = 2, 
} evrythng_log_level_t;


/** @brief Log callback prototype.
 */
typedef void (*evrythng_log_callback)(evrythng_log_level_t level, const char* fmt, va_list vl); 


/** @brief Pointer to internal context used by all the functions.
 */
typedef struct evrythng_ctx_t* evrythng_handle_t;


/** @brief Callback prototype.
 */
typedef void (*evrythng_callback)(); 


/** @brief Callback prototype used for subscribe functions,
 *  	   which is called on message arrival from the Evrythng
 *  	   cloud.
 *
 *  Note that str_json string  may not end with a \0 character.
 */
typedef void sub_callback(const char* str_json, size_t length);


/** @brief Initialize context.
 *
 * Use this function to initialize context which contains Evrythng client configuration
 * used for connecting to the Evrythng cloud. Use evrythng_destroy_handle to deinit it.
 *
 * @param[in] handle A pointer to context handle.
 *
 * @return    \b EVRYTHNG_BAD_ARGS     if handle is a null pointer \n
 *            \b EVRYTHNG_MEMORY_ERROR if an error occured while allocating memory \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngInitHandle(evrythng_handle_t* handle);


/** @brief Destroy context.
 *
 * Use this function to deinitialize context.
 *
 * @param[in] handle A context handle.
 *
 * @return void
 */
void EvrythngDestroyHandle(evrythng_handle_t handle);


/** @brief Set URL to connect to.
 *
 * Use this function to set URL to internal context, tcp://<ip>:<port> for 
 * unencrypted connection and ssl://<ip>:<port> for SSL.
 *
 * @param[in] handle A pointer to context handle.
 * @param[in] url A pointer to url.
 *
 * @return    \b EVRYTHNG_BAD_ARGS     if handle or url is null pointer or url does not start
 *                                     with a tcp or ssl \n
 *            \b EVRYTHNG_MEMORY_ERROR if an error occured while allocating memory \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngSetUrl(evrythng_handle_t handle, const char* url);


/** @brief Set API key to use for connecting to the cloud.
 *
 * Use this function to set API key to internal context 
 *
 * @param[in] handle A pointer to context handle.
 * @param[in] key A pointer to key.
 *
 * @return    \b EVRYTHNG_BAD_ARGS     if handle or key is null pointer \n
 *            \b EVRYTHNG_MEMORY_ERROR if an error occured while allocating memory \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngSetKey(evrythng_handle_t handle, const char* key);


/** @brief Set client id to use for connecting to the cloud.
 *
 * Use this function to set client id to internal context.
 * If client id was not setup a random 10 digits key will be generated.
 *
 * @param[in] handle A pointer to context handle.
 * @param[in] url A pointer to client id.
 *
 * @return    \b EVRYTHNG_BAD_ARGS     if handle or key is null pointer \n
 *            \b EVRYTHNG_MEMORY_ERROR if an error occured while allocating memory \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngSetClientId(evrythng_handle_t handle, const char* client_id);


/** @brief Set QoS to use for this connection.
 *
 * Use this function to set QoS.
 * If QoS was not setup a default value of 2 will be used.
 * See MQTT docs for more details.
 *
 * @param[in] handle A pointer to context handle.
 * @param[in] qos A Qos value.
 *
 * @return    \b EVRYTHNG_BAD_ARGS     if handle is a null pointer or QoS is > 2 or < 0 \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngSetQos(evrythng_handle_t handle, int qos);


/** @brief Set log callback
 *
 * Use this function to set log callback to internal context 
 * otherwise a null pointer will be setup.  One can use default 
 * internal log function with output to stdout.
 * To disable logging set 0 value or empty function.
 *
 * @param[in] handle A pointer to context handle.
 * @param[in] callback A pointer to log callback.
 *
 * @return    \b EVRYTHNG_BAD_ARGS     if handle or callback is a null pointer \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngSetLogCallback(evrythng_handle_t handle, evrythng_log_callback callback);


/** @brief Set callback on connection lost/restored
 *
 * Use this function to set callback that will be called on connection lost.
 *
 * @param[in] handle A pointer to context handle.
 * @param[in] on_connection_lost A pointer to callback to call when connection is lost.
 * @param[in] on_connection_restored A pointer to callback to call when connection is restored.
 *
 * @return    \b EVRYTHNG_BAD_ARGS     if handle or callback is a null pointer \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngSetConnectionCallbacks(evrythng_handle_t handle, 
        evrythng_callback on_connection_lost, evrythng_callback on_connection_restored);


/** @brief Set internal thread priority.
 *
 * Use this function to set internal thread priority.
 * If it was not setup a default value of 0 will be used.
 *
 * @param[in] handle A pointer to context handle.
 * @param[in] priority A priority value.
 *
 * @return    \b EVRYTHNG_BAD_ARGS     if handle is a null pointer or priority is < 0 \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngSetThreadPriority(evrythng_handle_t handle, int priority);


/** @brief Set internal thread stack size.
 *
 * Use this function to set internal thread stack size.
 * If it was not setup a default value of 4096 will be used.
 *
 * @param[in] handle A pointer to context handle.
 * @param[in] stacksize A stack size value.
 *
 * @return    \b EVRYTHNG_BAD_ARGS     if handle is a null pointer or stack size is < 1024 \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngSetThreadStacksize(evrythng_handle_t handle, int stacksize);


/** @brief Connect to Evrythng cloud.
 *
 * Use this function to connect to the Evrythng cloud.
 *
 * @param[in] handle    A handle to context which contains Evrythng client configuration
 *                      used for connecting to the Evrythng cloud.
 *
 * @return    \b EVRYTHNG_BAD_ARGS if handle is a null pointer \n
 *            \b EVRYTHNG_CERT_REQUIRED_ERROR if SSL url is setup but certificate was not provided\n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_FAILURE if mqtt client creation/setup error occured \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_CONNECTION_FAILED if could not establish connection to the cloud \n
 *            \b EVRYTHNG_AUTH_FAILED bad api key provided, server did not authorize the client \n
 *            \b EVRYTHNG_CLIENT_ID_REJECTED bad client id provided, server rejected it\n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngConnect(evrythng_handle_t handle);


/** @brief Disconnect from Evrythng cloud.
 *
 * Use this function to disconnect from Evrythng cloud.
 *
 * @param[in] handle    A handle to context which contains Evrythng client configuration
 *                      used for connecting to the Evrythng cloud.
 *
 * @return    \b EVRYTHNG_BAD_ARGS if handle is a null pointer \n
 *            \b EVRYTHNG_FAILURE if mqtt client creation/setup error occured \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS      on success \n
 */
evrythng_return_t EvrythngDisconnect(evrythng_handle_t handle);


/** @brief Publish a single property to a given thing.
 *
 * This function attempts to publish a single property to a given thing.
 *
 * @param[in] handle        A context handle.
 * @param[in] thng_id A     A thing ID.
 * @param[in] property_name The name of the property.
 * @param[in] property_json A JSON string which contains property value. 
 * 
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubThngProperty(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name, 
        const char* property_json);


/** @brief Subscribe to a single property of the thing.
 *
 * This function attempts to subscribe to a single property of the thing.
 *  
 * @param[in] handle        A context handle.
 * @param[in] thng_id       A thing ID.
 * @param[in] property_name The name of the property. 
 * @param[in] pub_states    The pubStates flag. 
 * @param[in] callback      A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubThngProperty(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe a client from a single property of the thing.
 *
 * This function unsubscribes a client from a single property of the thing. 
 *  
 * @param[in] handle        A context handle.
 * @param[in] thng_id       A thing ID.
 * @param[in] property_name The name of the property. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubThngProperty(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name);



/** @brief Subscribe to all properties of the thing.
 * 
 * This function subscribes to all properties of the thing.
 *
 * @param[in] handle   A context handle.
 * @param[in] thng_id  A thing ID. 
 * @param[in] pub_states    The pubStates flag. 
 * @param[in] callback A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubThngProperties(
        evrythng_handle_t handle, 
        const char* thng_id, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe a client from all properties of the thing.
 * 
 * This function attempts to unsubscribe a client from all properties of the thing. 
 *
 * @param[in] handle   A context handle.
 * @param[in] thng_id  A thing ID. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubThngProperties(
        evrythng_handle_t handle, 
        const char* thng_id);


/** @brief Publish a few properties to a given thing.
 *
 * This function attempts to publish a few properties to a given thing.
 *
 * @param[in] handle          A context handle.
 * @param[in] thng_id         A thing ID.
 * @param[in] properties_json A JSON string which contains properties values. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubThngProperties(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* properties_json);


/** @brief Subscribe to a single action of the thing.
 *
 * This function attempts to subscribe to a single action of the thing.
 *
 * @param[in] handle      A context handle.
 * @param[in] thng_id     A thing ID.
 * @param[in] action_name The name of an action. 
 * @param[in] pub_states    The pubStates flag. 
 * @param[in] callback    A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubThngAction(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* action_name, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe a client from a single action of the thing.
 *
 * This function unsubscribes a client from a single action of the thing. 
 *
 * @param[in] handle      A context handle.
 * @param[in] thng_id     A thing ID.
 * @param[in] action_name The name of an action. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubThngAction(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* action_name);


/** @brief Subscribe to all actions of the thing.
 *
 * This function attempts to subscribe to all actions of the thing.
 *
 * @param[in] handle   A context handle.
 * @param[in] thng_id  A thing ID. 
 * @param[in] pub_states The pubStates flag. 
 * @param[in] callback A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubThngActions(
        evrythng_handle_t handle, 
        const char* thng_id, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe a client from all actions of the thing.
 *
 * This function unsubscribes a client from all actions of the thing. 
 *
 * @param[in] handle   A context handle.
 * @param[in] thng_id  A thing ID. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubThngActions(
        evrythng_handle_t handle, 
        const char* thng_id);


/** @brief Publish a single action to a given thing. 
 *
 * This function attempts to publish a single action to a given thing. 
 *
 * @param[in] handle      A context handle.
 * @param[in] thng_id     A thing ID.
 * @param[in] action_name The name of an action.
 * @param[in] action_json A JSON string which contains an action. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubThngAction(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* action_name, 
        const char* action_json);


/** @brief Publish a few actions to a given thing.
 *
 * This function attempts to publish a few actions to a given thing.
 *
 * @param[in] handle       A context handle.
 * @param[in] thng_id      A thing ID.
 * @param[in] actions_json A JSON string which contains actions. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubThngActions(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* actions_json);


/** @brief Subscribe to a location of the thing.
 *
 * This function attempts to subscribe to a location of the thing.
 *
 * @param[in] handle   A context handle.
 * @param[in] thng_id  A thing ID. 
 * @param[in] pub_states    The pubStates flag. 
 * @param[in] callback A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubThngLocation(
        evrythng_handle_t handle, 
        const char* thng_id, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe a client from a location of the thing.
 *
 * This function unsubscribes a client from a location of the thing. 
 *
 * @param[in] handle     A context handle.
 * @param[in] thng_id    A thing ID. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubThngLocation(
        evrythng_handle_t handle, 
        const char* thng_id);


/** @brief Publish a location to a given thing.
 *
 * This function attempts to publish a location to a given thing.
 *
 * @param[in] handle        A context handle.
 * @param[in] thng_id       A thing ID.
 * @param[in] location_json A JSON string which contains location. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubThngLocation(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* location_json);


/** @brief Subscribe to a single property of the product.
 *
 * This function attempts to subscribe to a single property of the product.
 *
 * @param[in] handle        A context handle.
 * @param[in] product_id    A product ID.
 * @param[in] property_name The name of the property. 
 * @param[in] pub_states    The pubStates flag. 
 * @param[in] callback      A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubProductProperty(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* property_name, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe from a single property of the product.
 *
 * This function unsubscribes to a single property of the product.
 *
 * @param[in] handle        A context handle.
 * @param[in] product_id    A product ID.
 * @param[in] property_name The name of the property. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubProductProperty(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* property_name);



/** @brief Subscribe to all properties of the product.
 *
 * This function attempts to subscribe to all properties of the product.
 *
 * @param[in] handle      A context handle.
 * @param[in] product_id A product ID. 
 * @param[in] pub_states    The pubStates flag. 
 * @param[in] callback    A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubProductProperties(
        evrythng_handle_t handle, 
        const char* product_id, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe from all properties of the product.
 *
 * This function unsubscribes from all properties of the product.
 *
 * @param[in] handle        A context handle.
 * @param[in] product_id    A product ID.
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubProductProperties(
        evrythng_handle_t handle, 
        const char* product_id);


/** @brief Publish a single property to a given product.
 *
 * This function attempts to publish a single property to a given product.
 *
 * @param[in] handle        A context handle.
 * @param[in] product_id    A product ID.
 * @param[in] property_name The name of the property.
 * @param[in] property_json A JSON string which contains property value. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubProductProperty(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* property_name, 
        const char* property_json);


/** @brief Publish a few properties to a given product.
 *
 * This function attempts to publish a few properties to a given product.
 *
 * @param[in] handle          A context handle.
 * @param[in] product_id      A product ID.
 * @param[in] properties_json A JSON string which contains properties values. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubProductProperties(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* properties_json);


/** @brief Subscribe to a single action of the product.
 *
 * This function attempts to subscribe to a single action of the product.
 *
 * @param[in] handle      A context handle.
 * @param[in] product_id  A product ID.
 * @param[in] action_name The name of an action. 
 * @param[in] pub_states    The pubStates flag. 
 * @param[in] callback    A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubProductAction(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* action_name, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe from a single action of the product.
 *
 * This function unsubscribes from a single action of the product.
 *
 * @param[in] handle      A context handle.
 * @param[in] product_id  A product ID.
 * @param[in] action_name The name of an action. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubProductAction(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* action_name);


/** @brief Subscribe to all actions of the product.
 *
 * This function attempts to subscribe to all actions of the product.
 *
 * @param[in] handle      A context handle.
 * @param[in] product_id  A product ID. 
 * @param[in] pub_states  A pubStates flag. 
 * @param[in] callback    A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubProductActions(
        evrythng_handle_t handle, 
        const char* product_id, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe from all actions of the product.
 *
 * This function unsubscribes from all actions of the product.
 *
 * @param[in] handle      A context handle.
 * @param[in] product_id  A product ID.
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubProductActions(
        evrythng_handle_t handle, 
        const char* product_id);


/** @brief Publish a single action to a given product.
 *
 * This function attempts to publish a single action to a given product. 
 *
 * @param[in] handle      A context handle.
 * @param[in] product_id  A product ID.
 * @param[in] action_name The name of an action.
 * @param[in] action_json A JSON string which contains an action. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubProductAction(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* action_name, 
        const char* action_json);


/** @brief Publish a few actions to a given product.
 *
 * This function attempts to publish a few actions to a given product.
 *
 * @param[in] handle       A context handle.
 * @param[in] product_id   A product ID.
 * @param[in] actions_json A JSON string which contains actions. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubProductActions(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* actions_json);


/** @brief Subscribe to a single action.
 *
 * This function attempts to subscribe to a single action.
 *
 * @param[in] handle      A context handle.
 * @param[in] action_name The name of an action. 
 * @param[in] pub_states  A pubStates flag. 
 * @param[in] callback    A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubAction(
        evrythng_handle_t handle, 
        const char* action_name, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe from a single action.
 *
 * This function unsubscribes from a single action.
 *
 * @param[in] handle      A context handle.
 * @param[in] action_name The name of an action. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubAction(
        evrythng_handle_t handle, 
        const char* action_name);


/** @brief Subscribe to all actions.
 *
 * This function attempts to subscribe to all actions.
 *  
 * @param[in] handle   A context handle.
 * @param[in] pub_states A pubStates flag. 
 * @param[in] callback A pointer to a subscribe callback function. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_SUBSCRIPTION_ERROR if an error occured trying to subscribe to a topic \n
 *            \b EVRYTHNG_ALREADY_SUBSCRIBED if subcribtion already exists \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngSubActions(
        evrythng_handle_t handle, 
        int pub_states,
        sub_callback *callback);


/** @brief Unsubscribe from all actions.
 *
 * This function unsubscribes from all actions.
 *
 * @param[in] handle      A context handle.
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_UNSUBSCRIPTION_ERROR if an error occured trying to unsubscribe from a topic \n
 *            \b EVRYTHNG_NOT_SUBSCRIBED if trying to unsubcribe from an unexistent subscribtion \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngUnsubActions(
        evrythng_handle_t handle);


/** @brief Publish a single action.
 *
 * This function attempts to publish a single action.
 *
 * @param[in] handle      A context handle.
 * @param[in] action_name The name of an action.
 * @param[in] action_json A JSON string which contains an action. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubAction(
        evrythng_handle_t handle, 
        const char* action_name, 
        const char* action_json);


/** @brief Publish a few actions.
 *
 * This function attempts to publish a few actions.
 *
 * @param[in] handle       A context handle.
 * @param[in] actions_json A JSON string which contains actions. 
 *
 * @return    \b EVRYTHNG_BAD_ARGS if one the arguments is a null pointer or a too long string \n
 *            \b EVRYTHNG_PUBLISH_ERROR if an error occured trying to publish a message \n
 *            \b EVRYTHNG_MEMORY_ERROR if memory allocation error occured \n
 *            \b EVRYTHNG_NOT_CONNECTED if internal context is not in connected state \n
 *            \b EVRYTHNG_TIMEOUT timeout waiting for server response \n
 *            \b EVRYTHNG_SUCCESS on success \n
 */
evrythng_return_t EvrythngPubActions(
        evrythng_handle_t handle, 
        const char* actions_json);


#endif //_EVRYTHNG_H
