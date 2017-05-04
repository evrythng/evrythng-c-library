/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 */

#include <string.h>

#include "evrythng/evrythng.h"
#include "evrythng_config.h"
#include "CuTest.h"

#define PROPERTY_VALUE_JSON "[{\"value\": 500}]"
#define PROPERTIES_VALUE_JSON "[{\"key\": \"property_1\", \"value\": 500}, {\"key\": \"property_2\", \"value\": 100}]"
#define ACTION_JSON "{\"type\": \"_action_1\"}"
#define LOCATION_JSON  "[{\"position\": { \"type\": \"Point\", \"coordinates\": [-17.3, 36] }}]"

static Semaphore sub_sem;

void on_connection_lost()
{
    platform_printf("evt lib connection lost\n");
}

void on_connection_restored()
{
    platform_printf("evt lib connection restored\n");
}


void log_callback(evrythng_log_level_t level, const char* fmt, va_list vl)
{
    char msg[512];

    unsigned n = vsnprintf(msg, sizeof msg, fmt, vl);
    if (n >= sizeof msg)
        msg[sizeof msg - 1] = '\0';

    switch (level)
    {
        case EVRYTHNG_LOG_ERROR:
            platform_printf("ERROR: ");
            break;
        case EVRYTHNG_LOG_WARNING:
            platform_printf("WARNING: ");
            break;
        default:
        case EVRYTHNG_LOG_DEBUG:
            platform_printf("DEBUG: ");
            break;
    }
    platform_printf("%s\n", msg);
}

#define START_SINGLE_CONNECTION \
    PRINT_START_MEM_STATS \
    evrythng_handle_t h1;\
    common_tcp_init_handle(&h1);\
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));

#define END_SINGLE_CONNECTION \
    CuAssertIntEquals(tc, 0, platform_semaphore_wait(&sub_sem, 10000));\
    EvrythngDisconnect(h1);\
    EvrythngDestroyHandle(h1);\
    PRINT_END_MEM_STATS


#if defined(CONFIG_OS_FREERTOS) && !defined(FREERTOS_SIMULATOR)
#define PRINT_START_MEM_STATS \
    const heapAllocatorInfo_t* s1 = getheapAllocInfo();\
    wmprintf(">>>>>> %d/%d: %s\n\r", s1->freeSize, s1->heapSize, __func__);
#else
#define PRINT_START_MEM_STATS
#endif

#if defined(CONFIG_OS_FREERTOS) && !defined(FREERTOS_SIMULATOR)
#define PRINT_END_MEM_STATS \
    const heapAllocatorInfo_t* s2 = getheapAllocInfo();\
    wmprintf("<<<<<< %d/%d: %s\n\r", s2->freeSize, s2->heapSize, __func__);
#else
#define PRINT_END_MEM_STATS 
#endif

#define CuAssertIntInterval(tc, a, b, ret) \
    CuAssertIntEquals((tc), 1, ((ret) >= (a) && (ret) <= (b)))

#define CuAssertIntAlmostEqual(tc, expected, delta, ret) \
{\
    int _min = (expected) - (delta);\
    int _max = (expected) + (delta);\
    int _ret = (ret);\
    CuAssertIntInterval(tc, _min, _max, _ret);\
}


void test_init_handle_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngInitHandle(&h));
    EvrythngDestroyHandle(h);
}

void test_init_handle_fail(CuTest* tc)
{
    CuAssertIntEquals(tc, EVRYTHNG_BAD_ARGS, EvrythngInitHandle(0));
}

void test_set_url_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngInitHandle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetUrl(h, "tcp://localhost:666"));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetUrl(h, "ssl://localhost:666"));
    EvrythngDestroyHandle(h);
}

void test_set_url_fail(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngInitHandle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_BAD_ARGS, EvrythngSetUrl(h, 0));
    CuAssertIntEquals(tc, EVRYTHNG_BAD_URL, EvrythngSetUrl(h, "ttt://localhost:666"));
    EvrythngDestroyHandle(h);
}

void test_set_key_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngInitHandle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetKey(h, "123456789"));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetKey(h, "123456789123456789"));
    EvrythngDestroyHandle(h);
}

void test_set_client_id_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngInitHandle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetClientId(h, "123456789"));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetClientId(h, "123456789123456789"));
    EvrythngDestroyHandle(h);
}

void test_set_qos_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngInitHandle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetQos(h, 0));
    EvrythngDestroyHandle(h);
}

void test_set_qos_fail(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngInitHandle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_BAD_ARGS, EvrythngSetQos(h, 8));
    EvrythngDestroyHandle(h);
}

void test_set_callback_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngInitHandle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetLogCallback(h, log_callback));
    EvrythngDestroyHandle(h);
}

void test_set_callback_fail(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngInitHandle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetLogCallback(h, 0));
    EvrythngDestroyHandle(h);
}

static void common_tcp_init_handle(evrythng_handle_t* h)
{
    EvrythngInitHandle(h);
    EvrythngSetUrl(*h, MQTT_URL);
    EvrythngSetLogCallback(*h, log_callback);
    EvrythngSetConnectionCallbacks(*h, on_connection_lost, on_connection_restored);
    EvrythngSetKey(*h, DEVICE_API_KEY);
}

void test_tcp_connect_ok1(CuTest* tc)
{
    evrythng_handle_t h;
    common_tcp_init_handle(&h);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h));
    EvrythngDestroyHandle(h);
}

void test_tcp_connect_ok2(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h1));
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

static void test_sub_callback(const char* str_json, size_t len)
{
    char msg[len+1]; snprintf(msg, sizeof msg, "%s", str_json);
    platform_printf("%s: %s\n\r", __func__, msg);
    platform_semaphore_post(&sub_sem);
}

void test_unsub_nonexistent(CuTest* tc)
{
    PRINT_START_MEM_STATS
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));
    
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_NOT_SUBSCRIBED, EvrythngUnsubThngProperty(h1, THNG_1, PROPERTY_1));
    CuAssertIntEquals(tc, EVRYTHNG_NOT_SUBSCRIBED, EvrythngUnsubThngProperty(h1, THNG_1, PROPERTY_2));

    EvrythngDisconnect(h1);
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_sub_alreadysub(CuTest* tc)
{
    PRINT_START_MEM_STATS
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));
    
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngProperty(h1, THNG_1, PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_ALREADY_SUBSCRIBED, EvrythngSubThngProperty(h1, THNG_1, PROPERTY_1, 0, test_sub_callback));

    EvrythngDisconnect(h1);
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_sub_diffpubstate(CuTest* tc)
{
    PRINT_START_MEM_STATS
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));
    
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngProperty(h1, THNG_1, PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_ALREADY_SUBSCRIBED, EvrythngSubThngProperty(h1, THNG_1, PROPERTY_1, 1, test_sub_callback));

    EvrythngDisconnect(h1);
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_subunsub_thng(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);

    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));

    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngAction(h1, THNG_1, ACTION_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngAction(h1, THNG_1, ACTION_2, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubThngAction(h1, THNG_1, ACTION_2));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngProperty(h1, THNG_1, PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubThngProperty(h1, THNG_1, PROPERTY_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngProperty(h1, THNG_1, PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngProperty(h1, THNG_1, PROPERTY_2, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubThngProperty(h1, THNG_1, PROPERTY_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngProperties(h1, THNG_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubThngProperties(h1, THNG_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngActions(h1, THNG_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubThngActions(h1, THNG_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngLocation(h1, THNG_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubThngLocation(h1, THNG_1));

    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h1));
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_subunsub_prod(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));

    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductProperty(h1, PRODUCT_1, PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductProperty(h1, PRODUCT_1, PROPERTY_2, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubProductProperty(h1, PRODUCT_1, PROPERTY_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubProductProperty(h1, PRODUCT_1, PROPERTY_2));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductProperties(h1, PRODUCT_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubProductProperties(h1, PRODUCT_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductAction(h1, PRODUCT_1, ACTION_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductAction(h1, PRODUCT_1, ACTION_2, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubProductAction(h1, PRODUCT_1, ACTION_2));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductActions(h1, PRODUCT_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubProductActions(h1, PRODUCT_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubAction(h1, ACTION_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubAction(h1, ACTION_2, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngUnsubAction(h1, ACTION_2));

    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h1));
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_pubsub_thng_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngProperty(h1, THNG_1, PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubThngProperty(h1, THNG_1, PROPERTY_1, PROPERTY_VALUE_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_thng_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngProperties(h1, THNG_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubThngProperties(h1, THNG_1, PROPERTIES_VALUE_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_thng_action(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngAction(h1, THNG_1, ACTION_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubThngAction(h1, THNG_1, ACTION_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_thng_actions(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngActions(h1, THNG_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubThngActions(h1, THNG_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_thng_location(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngLocation(h1, THNG_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubThngLocation(h1, THNG_1, LOCATION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_prod_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductProperty(h1, PRODUCT_1, PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubProductProperty(h1, PRODUCT_1, PROPERTY_1, PROPERTY_VALUE_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_prod_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductProperties(h1, PRODUCT_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubProductProperties(h1, PRODUCT_1, PROPERTIES_VALUE_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_prod_action(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductAction(h1, PRODUCT_1, ACTION_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubProductAction(h1, PRODUCT_1, ACTION_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_prod_actions(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductActions(h1, PRODUCT_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubProductActions(h1, PRODUCT_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_action(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubAction(h1, ACTION_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubAction(h1, ACTION_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_actions(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubActions(h1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubActions(h1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_connect_bad_clientid(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetClientId(h1, ""));
    CuAssertIntEquals(tc, EVRYTHNG_CLIENT_ID_REJECTED, EvrythngConnect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h1));
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_connect_bad_apikey(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSetKey(h1, "123"));
    CuAssertIntEquals(tc, EVRYTHNG_AUTH_FAILED, EvrythngConnect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h1));
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_subscribe_bad_thngid(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubThngProperty(h1, "thng", PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubThngProperties(h1, "thng", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubThngAction(h1, "thng", ACTION_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubThngActions(h1, "rkuej", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubThngLocation(h1, "thng", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubProductProperty(h1, "product", PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubProductProperties(h1, "product", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubProductAction(h1, "product", PROPERTY_1, 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubProductActions(h1, "product", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h1));
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_subscribe_bad_entity_name(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubThngProperty(h1, THNG_1, "prop", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubThngAction(h1, THNG_1, "act", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubProductProperty(h1, PRODUCT_1, "prop", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUBSCRIPTION_ERROR, EvrythngSubProductAction(h1, PRODUCT_1, "prop", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngSubAction(h1, "act", 0, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h1));
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_publish_bad_thngid(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubThngProperty(h1, "rt", PROPERTY_1, PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubThngProperties(h1, "rt", PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubThngAction(h1, "rt", ACTION_1, PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubThngActions(h1, "rt", PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubThngLocation(h1, "rt", PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubProductProperty(h1, "rt", PROPERTY_1, PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubProductProperties(h1, "rt", PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubProductAction(h1, "rt", ACTION_1, PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubProductActions(h1, "rt", PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h1));
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}

void test_publish_bad_entity(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngConnect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubThngProperty(h1, THNG_1, "new", PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubThngAction(h1, THNG_1, "act", PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngPubProductProperty(h1, PRODUCT_1, "new", PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_PUBLISH_ERROR, EvrythngPubProductAction(h1, PRODUCT_1, "act", PROPERTY_VALUE_JSON));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, EvrythngDisconnect(h1));
    EvrythngDestroyHandle(h1);
    PRINT_END_MEM_STATS
}


extern int next_sleep_time(int);

static int next_time_cal_avg(int step)
{
    int acc = 0;
    int count = 10000;

    for (int i = 0; i < count; ++i) {
        acc += next_sleep_time(step);
    }

    //printf("avg sleep time on step %d = %d\n", step, acc/count);

    return acc/count;
}

void test_exp_backoff(CuTest* tc)
{
    srand(time(0));

    static int base = 500;
    static int min_sleep = 1000;//2*base;
#define next_time_calc(retry) ((1<<((retry)+2))*base)

    CuAssertIntEquals(tc, 0, next_sleep_time(0));

    CuAssertIntAlmostEqual(tc, next_time_calc(1)/2, 1000, next_time_cal_avg(1));
    CuAssertIntAlmostEqual(tc, next_time_calc(2)/2, 1000, next_time_cal_avg(2));
    CuAssertIntAlmostEqual(tc, next_time_calc(3)/2, 1000, next_time_cal_avg(3));
    CuAssertIntAlmostEqual(tc, next_time_calc(4)/2, 1000, next_time_cal_avg(4));
    CuAssertIntAlmostEqual(tc, next_time_calc(5)/2, 1000, next_time_cal_avg(5));
    CuAssertIntAlmostEqual(tc, next_time_calc(6)/2, 1000, next_time_cal_avg(6));
    CuAssertIntAlmostEqual(tc, next_time_calc(7)/2, 1500, next_time_cal_avg(7));
}


CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();

#if 1
	SUITE_ADD_TEST(suite, test_connect_bad_clientid);
	SUITE_ADD_TEST(suite, test_connect_bad_apikey);

	SUITE_ADD_TEST(suite, test_subscribe_bad_thngid);
	SUITE_ADD_TEST(suite, test_subscribe_bad_entity_name);

	SUITE_ADD_TEST(suite, test_publish_bad_thngid);
	SUITE_ADD_TEST(suite, test_publish_bad_entity);
    
	SUITE_ADD_TEST(suite, test_init_handle_ok);
	SUITE_ADD_TEST(suite, test_init_handle_fail);
	SUITE_ADD_TEST(suite, test_set_url_ok);
	SUITE_ADD_TEST(suite, test_set_url_fail);
	SUITE_ADD_TEST(suite, test_set_key_ok);
	SUITE_ADD_TEST(suite, test_set_client_id_ok);
	SUITE_ADD_TEST(suite, test_set_qos_ok);
	SUITE_ADD_TEST(suite, test_set_qos_fail);
	SUITE_ADD_TEST(suite, test_set_callback_ok);
	SUITE_ADD_TEST(suite, test_set_callback_fail);
	SUITE_ADD_TEST(suite, test_tcp_connect_ok1);
    SUITE_ADD_TEST(suite, test_tcp_connect_ok2);

	SUITE_ADD_TEST(suite, test_unsub_nonexistent);
	SUITE_ADD_TEST(suite, test_sub_alreadysub);
	SUITE_ADD_TEST(suite, test_sub_diffpubstate);

	SUITE_ADD_TEST(suite, test_subunsub_thng);
	SUITE_ADD_TEST(suite, test_subunsub_prod);

	SUITE_ADD_TEST(suite, test_pubsub_thng_prop);
	SUITE_ADD_TEST(suite, test_pubsuball_thng_prop);

	SUITE_ADD_TEST(suite, test_pubsub_thng_action);
	SUITE_ADD_TEST(suite, test_pubsuball_thng_actions);

	SUITE_ADD_TEST(suite, test_pubsub_thng_location);
	SUITE_ADD_TEST(suite, test_pubsub_prod_prop);

	SUITE_ADD_TEST(suite, test_pubsuball_prod_prop);
	SUITE_ADD_TEST(suite, test_pubsub_prod_action);
	SUITE_ADD_TEST(suite, test_pubsuball_prod_actions);

	SUITE_ADD_TEST(suite, test_pubsub_action);
	SUITE_ADD_TEST(suite, test_pubsuball_actions);

#endif
	SUITE_ADD_TEST(suite, test_exp_backoff);

	return suite;
}


void RunAllTests()
{
    platform_semaphore_init(&sub_sem);

	CuString *output = CuStringNew();
    CuSuite* suite = CuGetSuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
    platform_printf("%s\n", output->buffer);
    CuStringDelete(output);
    CuSuiteDelete(suite);

    platform_semaphore_deinit(&sub_sem);
}

