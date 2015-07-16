#include "evrythng.h"
#include "platform.h"

#include <string.h>
#include "CuTest.h"

#if defined(NO_FILESYSTEM)
#include "evrythng_tls_certificate.h"
#endif

#include "evrythng_config.h"


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


#if defined(CONFIG_OS_FREERTOS) && !defined(FREERTOS_SIMULATOR)
xSemaphoreHandle pub_sem;
xSemaphoreHandle sub_sem;
#else
#include <semaphore.h>
sem_t pub_sem;
sem_t sub_sem;
#endif

void log_callback(evrythng_log_level_t level, const char* fmt, va_list vl)
{
    char msg[512];

    unsigned n = vsnprintf(msg, sizeof msg, fmt, vl);
    if (n >= sizeof msg)
        msg[sizeof msg - 1] = '\0';

    switch (level)
    {
        case EVRYTHNG_LOG_ERROR:
            printf("ERROR: ");
            break;
        case EVRYTHNG_LOG_WARNING:
            printf("WARNING: ");
            break;
        default:
        case EVRYTHNG_LOG_DEBUG:
            printf("DEBUG: ");
            break;
    }
    printf("%s\n\r", msg);
}

#define START_PUBSUB \
    PRINT_START_MEM_STATS \
    evrythng_handle_t h1;\
    evrythng_handle_t h2;\
    common_tcp_init_handle(&h1);\
    common_tcp_init_handle(&h2);\
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h1));\
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h2));

#define END_PUBSUB \
    CuAssertIntEquals(tc, 0, sem_timed_wait(&pub_sem));\
    CuAssertIntEquals(tc, 0, sem_timed_wait(&sub_sem));\
    evrythng_disconnect(h1);\
    evrythng_disconnect(h2);\
    evrythng_destroy_handle(h1);\
    evrythng_destroy_handle(h2);\
    PRINT_END_MEM_STATS


#define START_SINGLE_CONNECTION \
    PRINT_START_MEM_STATS \
    evrythng_handle_t h1;\
    common_tcp_init_handle(&h1);\
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h1));

#define END_SINGLE_CONNECTION \
    evrythng_disconnect(h1);\
    evrythng_destroy_handle(h1);\
    PRINT_END_MEM_STATS


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

void test_set_ca_ok(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_certificate(h, "123", 3));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_set_certificate(h, "1", 1));
    evrythng_destroy_handle(h);
}

void test_set_ca_fail(CuTest* tc)
{
    evrythng_handle_t h;
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_init_handle(&h));
    CuAssertIntEquals(tc, EVRYTHNG_BAD_ARGS, evrythng_set_certificate(h, "123", 0));
    CuAssertIntEquals(tc, EVRYTHNG_BAD_ARGS, evrythng_set_certificate(h, 0, 1));
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
    evrythng_set_key(*h, DEVICE_API_KEY);
#if defined(NO_FILESYSTEM)
    evrythng_set_certificate(*h, cert_buffer, sizeof(cert_buffer) - 1);
#else
    evrythng_set_certificate(*h, "demo/client.pem", strlen("demo/client.pem")+1);
#endif
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
    START_SINGLE_CONNECTION
    END_SINGLE_CONNECTION
}

void test_tcp_connect_ok3(CuTest* tc)
{
    START_SINGLE_CONNECTION
    evrythng_handle_t h2;
    evrythng_handle_t h3;
    common_tcp_init_handle(&h2);
    common_tcp_init_handle(&h3);
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h2));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_connect(h3));
    evrythng_disconnect(h2);
    evrythng_disconnect(h3);
    evrythng_destroy_handle(h2);
    evrythng_destroy_handle(h3);
    END_SINGLE_CONNECTION
}

static void test_pub_callback()
{
    printf("%s: message published\n\r", __func__);
#if defined(CONFIG_OS_FREERTOS) && !defined(FREERTOS_SIMULATOR)
    xSemaphoreGive(pub_sem);
#else
    sem_post(&pub_sem);
#endif
}

static void test_sub_callback(const char* str_json, size_t len)
{
    char msg[len+1]; snprintf(msg, sizeof msg, "%s", str_json);
    printf("%s: %s\n\r", __func__, msg);
#if defined(CONFIG_OS_FREERTOS) && !defined(FREERTOS_SIMULATOR)
    xSemaphoreGive(sub_sem);
#else
    sem_post(&sub_sem);
#endif
}

#if defined(CONFIG_OS_FREERTOS) && !defined(FREERTOS_SIMULATOR)
int sem_timed_wait(xSemaphoreHandle* sem)
{
    if (xSemaphoreTake(*sem, configTICK_RATE_HZ * 10) == pdTRUE)
        return 0;
    else
        return -1;
}
#else
int sem_timed_wait(sem_t* sem)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 10;

    do 
    {
        int ret = sem_timedwait(sem, &ts);
        if (!ret) return 0;
        else
        { 
            if (errno == EINTR) continue;
            return -1;
        }
    } while(1);

    return -1;
}
#endif

void test_subunsub_thng(CuTest* tc)
{
    START_SINGLE_CONNECTION
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
    END_SINGLE_CONNECTION
}

void test_subunsub_prod(CuTest* tc)
{
    START_SINGLE_CONNECTION
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
    END_SINGLE_CONNECTION
}

void test_pubsub_thng_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_property(h1, THNG_1, PROPERTY_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_property(h1, THNG_1, PROPERTY_1, PROPERTY_VALUE_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsuball_thng_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_properties(h1, THNG_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_properties(h1, THNG_1, PROPERTIES_VALUE_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsub_thng_action(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_action(h1, THNG_1, ACTION_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_action(h1, THNG_1, ACTION_1, ACTION_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsuball_thng_actions(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_actions(h1, THNG_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_actions(h1, THNG_1, ACTION_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsub_thng_location(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_thng_location(h1, THNG_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_thng_location(h1, THNG_1, LOCATION_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsub_prod_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_property(h1, PRODUCT_1, PROPERTY_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_product_property(h1, PRODUCT_1, PROPERTY_1, PROPERTY_VALUE_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsuball_prod_prop(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_properties(h1, PRODUCT_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_product_properties(h1, PRODUCT_1, PROPERTIES_VALUE_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsub_prod_action(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_action(h1, PRODUCT_1, ACTION_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_product_action(h1, PRODUCT_1, ACTION_1, ACTION_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsuball_prod_actions(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_product_actions(h1, PRODUCT_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_product_actions(h1, PRODUCT_1, ACTION_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsub_action(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_action(h1, ACTION_1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_action(h1, ACTION_1, ACTION_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

void test_pubsuball_actions(CuTest* tc)
{
    START_SINGLE_CONNECTION
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_subscribe_actions(h1, test_sub_callback));
    CuAssertIntEquals(tc, EVRYTHNG_SUCCESS, evrythng_publish_actions(h1, ACTION_JSON, test_pub_callback));
    END_SINGLE_CONNECTION
}

CuSuite* CuGetSuite(void)
{
	CuSuite* suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_init_handle_ok);
	SUITE_ADD_TEST(suite, test_init_handle_fail);
	SUITE_ADD_TEST(suite, test_destroy_handle);
	SUITE_ADD_TEST(suite, test_set_url_ok);
	SUITE_ADD_TEST(suite, test_set_url_fail);
	SUITE_ADD_TEST(suite, test_set_key_ok);
	SUITE_ADD_TEST(suite, test_set_client_id_ok);
	SUITE_ADD_TEST(suite, test_set_ca_ok);
	SUITE_ADD_TEST(suite, test_set_ca_fail);
	SUITE_ADD_TEST(suite, test_set_qos_ok);
	SUITE_ADD_TEST(suite, test_set_qos_fail);
	SUITE_ADD_TEST(suite, test_set_callback_ok);
	SUITE_ADD_TEST(suite, test_set_callback_fail);

	SUITE_ADD_TEST(suite, test_tcp_connect_ok1);
	SUITE_ADD_TEST(suite, test_tcp_connect_ok2);
	//SUITE_ADD_TEST(suite, test_tcp_connect_ok3);

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

	return suite;
}

void RunAllTests(void* v)
{
#if defined(CONFIG_OS_FREERTOS) && !defined(FREERTOS_SIMULATOR)
    vSemaphoreCreateBinary(pub_sem);
    vSemaphoreCreateBinary(sub_sem);
#else
    sem_init(&pub_sem, 0, 0);
    sem_init(&sub_sem, 0, 0);
#endif

	CuString *output = CuStringNew();
    CuSuite* suite = CuGetSuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
    printf("%s\n\r", output->buffer);
    CuStringDelete(output);
    CuSuiteDelete(suite);

#if defined(CONFIG_OS_FREERTOS)
    vTaskEndScheduler();
#endif
}

