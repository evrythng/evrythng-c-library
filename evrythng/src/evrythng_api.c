/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 */

#include "evrythng/evrythng.h"

evrythng_return_t evrythng_publish( evrythng_handle_t handle, const char* entity, 
        const char* entity_id, const char* data_type, const char* data_name, const char* property_json);

evrythng_return_t evrythng_subscribe( evrythng_handle_t handle, const char* entity, 
        const char* entity_id, const char* data_type, const char* data_name, 
        int pub_states, sub_callback *callback);

evrythng_return_t evrythng_unsubscribe( evrythng_handle_t handle, const char* entity, 
        const char* entity_id, const char* data_type, const char* data_name);

evrythng_return_t EvrythngPubThngProperty(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name, 
        const char* property_json)
{
    if (!thng_id || !property_name || !property_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "properties", property_name, property_json);
}


evrythng_return_t EvrythngSubThngProperty(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name, 
        int pub_states,
        sub_callback *callback)
{
    if (!thng_id || !property_name || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "properties", property_name, pub_states, callback);
}

evrythng_return_t EvrythngUnsubThngProperty(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name)
{
    if (!thng_id || !property_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "properties", property_name);
}


evrythng_return_t EvrythngSubThngProperties(
        evrythng_handle_t handle, 
        const char* thng_id, 
        int pub_states,
        sub_callback *callback)
{
    if (!thng_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "properties", NULL, pub_states, callback);
}


evrythng_return_t EvrythngUnsubThngProperties(
        evrythng_handle_t handle, 
        const char* thng_id)
{
    if (!thng_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "properties", NULL);
}


evrythng_return_t EvrythngPubThngProperties(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* properties_json)
{
    if (!thng_id || !properties_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "properties", NULL, properties_json);
}


evrythng_return_t EvrythngSubThngAction(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* action_name, 
        int pub_states,
        sub_callback *callback)
{
    if (!thng_id || !action_name || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "actions", action_name, pub_states, callback);
}


evrythng_return_t EvrythngUnsubThngAction(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* action_name)
{
    if (!thng_id || !action_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "actions", action_name);
}


evrythng_return_t EvrythngSubThngActions(
        evrythng_handle_t handle, 
        const char* thng_id, 
        int pub_states,
        sub_callback *callback)
{
    if (!thng_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "actions", "all", pub_states, callback);
}


evrythng_return_t EvrythngUnsubThngActions(
        evrythng_handle_t handle, 
        const char* thng_id)
{
    if (!thng_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "actions", "all");
}


evrythng_return_t EvrythngPubThngAction(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* action_name, 
        const char* action_json)
{
    if (!thng_id || !action_name || !action_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "actions", action_name, action_json);
}


evrythng_return_t EvrythngPubThngActions(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* actions_json)
{
    if (!thng_id || !actions_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "actions", "all", actions_json);
}


evrythng_return_t EvrythngSubThngLocation(
        evrythng_handle_t handle, 
        const char* thng_id, 
        int pub_states,
        sub_callback *callback)
{
    if (!thng_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "thngs", thng_id, "location", NULL, pub_states, callback);
}


evrythng_return_t EvrythngUnsubThngLocation(
        evrythng_handle_t handle, 
        const char* thng_id)
{
    if (!thng_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "thngs", thng_id, "location", NULL);
}


evrythng_return_t EvrythngPubThngLocation(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* location_json)
{
    if (!thng_id || !location_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "location", NULL, location_json);
}


evrythng_return_t EvrythngSubProductProperty(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* property_name, 
        int pub_states,
        sub_callback *callback)
{
    if (!product_id || !property_name || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "products", product_id, "properties", property_name, pub_states, callback);
}


evrythng_return_t EvrythngUnsubProductProperty(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* property_name)
{
    if (!product_id || !property_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "products", product_id, "properties", property_name);
}


evrythng_return_t EvrythngSubProductProperties(
        evrythng_handle_t handle, 
        const char* product_id, 
        int pub_states,
        sub_callback *callback)
{
    if (!product_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "products", product_id, "properties", NULL, pub_states, callback);
}


evrythng_return_t EvrythngUnsubProductProperties(
        evrythng_handle_t handle, 
        const char* product_id)
{
    if (!product_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "products", product_id, "properties", NULL);
}


evrythng_return_t EvrythngPubProductProperty(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* property_name, 
        const char* property_json)
{
    if (!product_id || !property_name || !property_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "properties", property_name, property_json);
}


evrythng_return_t EvrythngPubProductProperties(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* properties_json)
{
    if (!product_id || !properties_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "properties", NULL, properties_json);
}


evrythng_return_t EvrythngSubProductAction(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* action_name, 
        int pub_states,
        sub_callback *callback)
{
    if (!product_id || !action_name || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "products", product_id, "actions", action_name, pub_states, callback);
}


evrythng_return_t EvrythngUnsubProductAction(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* action_name)
{
    if (!product_id || !action_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "products", product_id, "actions", action_name);
}


evrythng_return_t EvrythngSubProductActions(
        evrythng_handle_t handle, 
        const char* product_id, 
        int pub_states,
        sub_callback *callback)
{
    if (!product_id || !callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "products", product_id, "actions", "all", pub_states, callback);
}


evrythng_return_t EvrythngUnsubProductActions(
        evrythng_handle_t handle, 
        const char* product_id)
{
    if (!product_id)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "products", product_id, "actions", "all");
}


evrythng_return_t EvrythngPubProductAction(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* action_name, 
        const char* action_json)
{
    if (!product_id || !action_name || !action_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "actions", action_name, action_json);
}


evrythng_return_t EvrythngPubProductActions(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* actions_json)
{
    if (!product_id || !actions_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "actions", "all", actions_json);
}


evrythng_return_t EvrythngSubAction(
        evrythng_handle_t handle, 
        const char* action_name, 
        int pub_states,
        sub_callback *callback)
{
    if (!action_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "actions", NULL, NULL, action_name, pub_states, callback);
}


evrythng_return_t EvrythngUnsubAction(
        evrythng_handle_t handle, 
        const char* action_name)
{
    if (!action_name)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_unsubscribe(handle, "actions", NULL, NULL, action_name);
}


evrythng_return_t EvrythngSubActions(evrythng_handle_t handle, int pub_states, sub_callback *callback)
{
    if (!callback)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_subscribe(handle, "actions", NULL, NULL, "all", pub_states, callback);
}


evrythng_return_t EvrythngUnsubActions(evrythng_handle_t handle)
{
    return evrythng_unsubscribe(handle, "actions", NULL, NULL, "all");
}


evrythng_return_t EvrythngPubAction(
        evrythng_handle_t handle, 
        const char* action_name, 
        const char* action_json)
{
    if (!action_name || !action_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "actions", NULL, NULL, action_name, action_json);
}


evrythng_return_t EvrythngPubActions(
        evrythng_handle_t handle, 
        const char* actions_json)
{
    if (!actions_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "actions", NULL, NULL, "all", actions_json);
}

