/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 */

#include "evrythng.h"

evrythng_return_t evrythng_publish( evrythng_handle_t handle, const char* entity, 
        const char* entity_id, const char* data_type, const char* data_name, const char* property_json);

evrythng_return_t evrythng_subscribe( evrythng_handle_t handle, const char* entity, 
        const char* entity_id, const char* data_type, const char* data_name, sub_callback *callback);

evrythng_return_t evrythng_unsubscribe( evrythng_handle_t handle, const char* entity, 
        const char* entity_id, const char* data_type, const char* data_name);

evrythng_return_t evrythng_publish_thng_property(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* property_name, 
        const char* property_json)
{
    if (!thng_id || !property_name || !property_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "properties", property_name, property_json);
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
        const char* properties_json)
{
    if (!thng_id || !properties_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "properties", NULL, properties_json);
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
        const char* action_json)
{
    if (!thng_id || !action_name || !action_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "actions", action_name, action_json);
}


evrythng_return_t evrythng_publish_thng_actions(
        evrythng_handle_t handle, 
        const char* thng_id, 
        const char* actions_json)
{
    if (!thng_id || !actions_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "actions", "all", actions_json);
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
        const char* location_json)
{
    if (!thng_id || !location_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "thngs", thng_id, "location", NULL, location_json);
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
        const char* property_json)
{
    if (!product_id || !property_name || !property_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "properties", property_name, property_json);
}


evrythng_return_t evrythng_publish_product_properties(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* properties_json)
{
    if (!product_id || !properties_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "properties", NULL, properties_json);
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
        const char* action_json)
{
    if (!product_id || !action_name || !action_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "actions", action_name, action_json);
}


evrythng_return_t evrythng_publish_product_actions(
        evrythng_handle_t handle, 
        const char* product_id, 
        const char* actions_json)
{
    if (!product_id || !actions_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "products", product_id, "actions", "all", actions_json);
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
        const char* action_json)
{
    if (!action_name || !action_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "actions", NULL, NULL, action_name, action_json);
}


evrythng_return_t evrythng_publish_actions(
        evrythng_handle_t handle, 
        const char* actions_json)
{
    if (!actions_json)
        return EVRYTHNG_BAD_ARGS;

    return evrythng_publish(handle, "actions", NULL, NULL, "all", actions_json);
}

