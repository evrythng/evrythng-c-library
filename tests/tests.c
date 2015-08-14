#include <string.h>

#include "evrythng.h"
#include "evrythng_config.h"
#include "CuTest.h"

#define PROPERTY_VALUE_JSON "[{\"value\": 500}]"
#define PROPERTIES_VALUE_JSON "[{\"key\": \"property_1\", \"value\": 500}, {\"key\": \"property_2\", \"value\": 100}]"
#define ACTION_JSON "{\"type\": \"_action_1\"}"
#define LOCATION_JSON  "[{\"position\": { \"type\": \"Point\", \"coordinates\": [-17.3, 36] }}]"


#if defined(FREERTOS_SIMULATOR)
#include <unistd.h>
/* This is application idle hook which is used by FreeRTOS */
void vApplicationIdleHook(void)
{
    sleep(1);
}
#endif

static int stop_cycling;
static Semaphore sub_sem;
static Thread t;


void evrythng_process(void* arg)
{
    platform_printf("%s STARTED\n", __func__);
    while (!stop_cycling)
    {
        evrythng_message_cycle((evrythng_handle_t)arg, 200);
        platform_sleep(100);
    }
    platform_printf("%s STOPPED\n", __func__);
}


void start_processing_thread(evrythng_handle_t handle)
{
    stop_cycling = 0;
    ThreadCreate(&t, 0, "mqtt_loop", evrythng_process, 8192, handle);
}


void conlost_callback(evrythng_handle_t handle)
{
    stop_cycling = 1;
}


void stop_processing_thread(evrythng_handle_t handle)
{
    stop_cycling = 1;
    ThreadJoin(&t, 10000);
    ThreadDestroy(&t);
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

#if 0

#define START_SINGLE_CONNECTION \
    PRINT_START_MEM_STATS \
    evrythng_handle_t h1;\
    common_tcp_init_handle(&h1);\
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h1));\
    start_processing_thread(h1);

#define END_SINGLE_CONNECTION \
    CuAssertIntEquals(tc, 0, SemaphoreWait(&sub_sem, 5000));\
    stop_processing_thread(h1);\
    evrythng_disconnect(h1);\
    evrythng_destroy_handle(h1);\
    PRINT_END_MEM_STATS

#else

#define START_SINGLE_CONNECTION \
    PRINT_START_MEM_STATS \
    evrythng_handle_t h1;\
    common_tcp_init_handle(&h1);\
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h1));\

#define END_SINGLE_CONNECTION \
    Timer t; TimerInit(&t);\
    TimerCountdownMS(&t, 5000);\
    int rc = 0;\
    do \
    {\
        evrythng_message_cycle(h1, 200);\
        platform_sleep(100);\
        rc = SemaphoreWait(&sub_sem, 0);\
        if (rc == 0)\
            break;\
    } while (TimerLeftMS(&t) > 0);\
    CuAssertIntEquals(tc, 0, rc);\
    evrythng_disconnect(h1);\
    evrythng_destroy_handle(h1);\
    TimerDeinit(&t);\
    PRINT_END_MEM_STATS

#endif


#if defined(CONFIG_OS_FREERTOS) && !defined(FREERTOS_SIMULATOR)
#define PRINT_START_MEM_STATS \
    const heapAllocatorInfo_t* s1 = getheapAllocInfo();\
    wmprintf(">>>>>> %d/%d\n\r", s1->freeSize, s1->heapSize);
#else
#define PRINT_START_MEM_STATS
#endif

#if defined(CONFIG_OS_FREERTOS) && !defined(FREERTOS_SIMULATOR)
#define PRINT_END_MEM_STATS \
    const heapAllocatorInfo_t* s2 = getheapAllocInfo();\
    wmprintf("<<<<<< %d/%d\n\r", s2->freeSize, s2->heapSize);
#else
#define PRINT_END_MEM_STATS 
#endif


void test_init_handle_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    evrythng_destroy_handle(h);
}

void test_init_handle_fail(CuTest* tc)
{
    CuAssertIntEquals(tc, EVRYTHNG_BAD_ARGS, evrythng_init_handle(0));
}

void test_destroy_handle(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    evrythng_destroy_handle(h);
}

void test_set_url_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_url(h, "tcp://localhost:666"));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_url(h, "ssl://localhost:666"));
    evrythng_destroy_handle(h);
}

void test_set_url_fail(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_BAD_ARGS, evrythng_set_url(h, 0));
    CuAssertIntEquals(tc, EVRYTHNG_BAD_URL, evrythng_set_url(h, "ttt://localhost:666"));
    evrythng_destroy_handle(h);
}

void test_set_key_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_key(h, "123456789"));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_key(h, "123456789123456789"));
    evrythng_destroy_handle(h);
}

void test_set_client_id_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_client_id(h, "123456789"));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_client_id(h, "123456789123456789"));
    evrythng_destroy_handle(h);
}

void test_set_qos_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_qos(h, 0));
    evrythng_destroy_handle(h);
}

void test_set_qos_fail(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_BAD_ARGS, evrythng_set_qos(h, 8));
    evrythng_destroy_handle(h);
}

void test_set_callback_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_log_callback(h, log_callback));
    evrythng_destroy_handle(h);
}

void test_set_callback_fail(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_log_callback(h, 0));
    evrythng_destroy_handle(h);
}

static void common_tcp_init_handle(evrythng_handle_t* h)
{
    evrythng_init_handle(h);
    evrythng_set_url(*h, MQTT_URL);
    evrythng_set_log_callback(*h, log_callback);
    evrythng_set_conlost_callback(*h, conlost_callback);
    evrythng_set_key(*h, DEVICE_API_KEY);
}

void test_tcp_connect_ok1(CuTest* tc)
{
    evrythng_handle_t h;
    common_tcp_init_handle(&h);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_disconnect(h));
    evrythng_destroy_handle(h);
}

void test_tcp_connect_ok2(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_disconnect(h1));
    evrythng_destroy_handle(h1);
    PRINT_END_MEM_STATS
}

static void test_sub_callback(const char* str_json, size_t len)
{
    char msg[len+1]; snprintf(msg, sizeof msg, "%s", str_json);
    platform_printf("%s: %s\n\r", __func__, msg);
    SemaphorePost(&sub_sem);
}


void test_subunsub_thng(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);

    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_action(h1, THNG_1, ACTION_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_action(h1, THNG_1, ACTION_2, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_thng_action(h1, THNG_1, ACTION_2));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_property(h1, THNG_1, PROPERTY_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_thng_property(h1, THNG_1, PROPERTY_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_property(h1, THNG_1, PROPERTY_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_property(h1, THNG_1, PROPERTY_2, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_thng_property(h1, THNG_1, PROPERTY_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_properties(h1, THNG_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_thng_properties(h1, THNG_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_actions(h1, THNG_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_thng_actions(h1, THNG_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_location(h1, THNG_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_thng_location(h1, THNG_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_disconnect(h1));

    evrythng_destroy_handle(h1);
    PRINT_END_MEM_STATS
}

void test_subunsub_prod(CuTest* tc)
{
    PRINT_START_MEM_STATS 
    evrythng_handle_t h1;
    common_tcp_init_handle(&h1);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h1));

    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_property(h1, PRODUCT_1, PROPERTY_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_property(h1, PRODUCT_1, PROPERTY_2, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_product_property(h1, PRODUCT_1, PROPERTY_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_product_property(h1, PRODUCT_1, PROPERTY_2));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_properties(h1, PRODUCT_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_product_properties(h1, PRODUCT_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_action(h1, PRODUCT_1, ACTION_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_action(h1, PRODUCT_1, ACTION_2, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_product_action(h1, PRODUCT_1, ACTION_2));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_actions(h1, PRODUCT_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_product_actions(h1, PRODUCT_1));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_action(h1, ACTION_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_action(h1, ACTION_2, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_unsubscribe_action(h1, ACTION_2));

    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_disconnect(h1));
    evrythng_destroy_handle(h1);
    PRINT_END_MEM_STATS
}

void test_pubsub_thng_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_property(h1, THNG_1, PROPERTY_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_property(h1, THNG_1, PROPERTY_1, PROPERTY_VALUE_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_thng_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_properties(h1, THNG_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_properties(h1, THNG_1, PROPERTIES_VALUE_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_thng_action(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_action(h1, THNG_1, ACTION_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_action(h1, THNG_1, ACTION_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_thng_actions(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_actions(h1, THNG_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_actions(h1, THNG_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_thng_location(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_location(h1, THNG_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_location(h1, THNG_1, LOCATION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_prod_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_property(h1, PRODUCT_1, PROPERTY_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_product_property(h1, PRODUCT_1, PROPERTY_1, PROPERTY_VALUE_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_prod_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_properties(h1, PRODUCT_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_product_properties(h1, PRODUCT_1, PROPERTIES_VALUE_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_prod_action(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_action(h1, PRODUCT_1, ACTION_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_product_action(h1, PRODUCT_1, ACTION_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_prod_actions(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_actions(h1, PRODUCT_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_product_actions(h1, PRODUCT_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsub_action(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_action(h1, ACTION_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_action(h1, ACTION_1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

void test_pubsuball_actions(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_actions(h1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_actions(h1, ACTION_JSON));
    END_SINGLE_CONNECTION
}

CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();

#if 1
	SUITE_ADD_TEST(suite, test_init_handle_ok);
	SUITE_ADD_TEST(suite, test_init_handle_fail);
	SUITE_ADD_TEST(suite, test_destroy_handle);
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

	SUITE_ADD_TEST(suite, test_subunsub_thng);
	SUITE_ADD_TEST(suite, test_subunsub_prod);
#endif

#if 1
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

	return suite;
}


void RunAllTests()
{
    SemaphoreInit(&sub_sem);

	CuString *output = CuStringNew();
    CuSuite* suite = CuGetSuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
    platform_printf("%s\n\r", output->buffer);
    CuStringDelete(output);
    CuSuiteDelete(suite);

    SemaphoreDeinit(&sub_sem);
}

