#if !defined(__MQTT_WICED_)
#define __MQTT_WICED_

#include "wiced.h"


typedef struct Timer
{
    wiced_time_t time;
} Timer;

typedef struct Network
{
    wiced_bool_t tls_enabled;
    wiced_tls_simple_context_t tls_context;
	wiced_tcp_socket_t socket;
    wiced_tcp_stream_t stream;
} Network;

typedef struct Mutex
{
	wiced_mutex_t mutex;
} Mutex;

typedef struct Semaphore
{
    wiced_semaphore_t sem;
} Semaphore;

typedef struct Thread
{
    wiced_thread_t tid;
    void* arg;
    void (*func)(void*);
} Thread;


#endif //__MQTT_WICED_
