#include "WICED/types.h"
#include <stdio.h>
#include <stdarg.h>
#include "evrythng_platform.h"

void TimerInit(Timer* t)
{
    t->time = 0;
}

void TimerDeinit(Timer* t)
{
}


char TimerIsExpired(Timer* t)
{
    wiced_time_t now;
    wiced_time_get_time(&now);
    return now >= t->time;
}


void TimerCountdownMS(Timer* t, unsigned int ms)
{
    wiced_time_get_time(&t->time);
    t->time += ms;
}


int TimerLeftMS(Timer* t)
{
    wiced_time_t now;
    wiced_time_get_time(&now);
    int64_t res = t->time - now;
    return res < 0 ? 0 : res;
}


void NetworkInit(Network* t)
{
}


int NetworkConnect(Network* n, char* hostname, int port)
{
    wiced_ip_address_t ip_address;
    wiced_result_t rc = -1;

    rc = wiced_hostname_lookup(hostname, &ip_address, 10000);
    if (rc != WICED_SUCCESS)
    {
        platform_printf("failed to resolve ip address of %s, rc = %d\n", hostname, rc);
        goto exit;
    }

    /* Create a TCP socket */
    rc = wiced_tcp_create_socket(&n->socket, WICED_STA_INTERFACE);
    if (rc != WICED_SUCCESS)
    {
        platform_printf("tcp socket creation failed, rc = %d\n", rc);
        goto exit;
    }

    //wiced_tcp_bind(&n->socket, WICED_ANY_PORT);

    rc = wiced_tcp_connect(&n->socket, &ip_address, port, 5000);
    if (rc != WICED_SUCCESS)
    {
        platform_printf("unable to establish connection to %s:%d, rc = %d\n", hostname, port, rc);
        goto exit;
    }

    rc = wiced_tcp_stream_init(&n->stream, &n->socket);
    if (rc != WICED_SUCCESS)
    {
        platform_printf("unable to init tcp stream, rc = %d\n", rc);
        goto exit;
    }

    rc = WICED_SUCCESS;
exit:
    if (rc != WICED_SUCCESS)
        NetworkDisconnect(n);
    return rc;
}


void NetworkDisconnect(Network* n)
{
    wiced_tcp_disconnect(&n->socket);
    wiced_tcp_stream_deinit(&n->stream);
    wiced_tcp_delete_socket(&n->socket);
}


int NetworkRead(Network* n, unsigned char* buffer, int length, int timeout)
{
    wiced_result_t rc = wiced_tcp_stream_read(&n->stream, buffer, length, timeout);
    if (rc != WICED_SUCCESS)
    {
        if (rc == WICED_TIMEOUT)
        {
            return -1;
        }
        else
        {
            platform_printf("failed to read data from tcp stream, rc = %d\n", rc);
            return 0;
        }
    }

    //platform_printf("successfully read %d bytes from tcp stream\n", length);

    return length;
}


int NetworkWrite(Network* n, unsigned char* buffer, int length, int timeout)
{
    wiced_result_t rc = wiced_tcp_stream_write(&n->stream, buffer, length);
    if (rc != WICED_SUCCESS)
    {
        platform_printf("unable to write data to tcp stream, rc = %d\n", rc);
        return -1;
    }

    rc = wiced_tcp_stream_flush(&n->stream);
    if (rc != WICED_SUCCESS)
    {
        platform_printf("unable to flush tcp stream, rc = %d\n", rc);
        return -1;
    }

    //platform_printf("%s: successfully sent %d bytes to tcp stream\n", __func__, length);

    return length;
}


void MutexInit(Mutex* m)
{
    if (!m)
    {
        platform_printf("%s: invalid mutex %p\n", __func__, m);
        return;
    }

    wiced_rtos_init_mutex(&m->mutex);
}


int MutexLock(Mutex* m)
{
    if (!m)
    {
        platform_printf("%s: invalid mutex %p\n", __func__, m);
        return -1;
    }

    wiced_result_t rc = wiced_rtos_lock_mutex(&m->mutex);
    if (rc != WICED_SUCCESS)
        platform_printf("%s: FAILED to lock %p: %d\n", __func__, &m->mutex, rc);
    return 0;
}


int MutexUnlock(Mutex* m)
{
    if (!m)
    {
        platform_printf("%s: invalid mutex %p\n", __func__, m);
        return -1;
    }

    wiced_result_t rc = wiced_rtos_unlock_mutex(&m->mutex);
    if (rc != WICED_SUCCESS)
        platform_printf("%s: FAILED to unlock %p: %d\n", __func__, &m->mutex, rc);
    return 0;
}


void MutexDeinit(Mutex* m)
{
    if (!m)
    {
        platform_printf("%s: invalid mutex %p\n", __func__, m);
        return;
    }

    wiced_rtos_deinit_mutex(&m->mutex);
}


void SemaphoreInit(Semaphore* s)
{
    if (!s)
    {
        platform_printf("%s: invalid semaphore %p\n", __func__, s);
        return;
    }

    wiced_rtos_init_semaphore(&s->sem);
}


void SemaphoreDeinit(Semaphore* s)
{
    if (!s)
    {
        platform_printf("%s: invalid semaphore %p\n", __func__, s);
        return;
    }

    wiced_rtos_deinit_semaphore(&s->sem);
}


int SemaphorePost(Semaphore* s)
{
    if (!s)
    {
        platform_printf("%s: invalid semaphore %p\n", __func__, s);
        return -1;
    }

    wiced_result_t rc = wiced_rtos_set_semaphore(&s->sem);
    if (rc != WICED_SUCCESS)
    {
        platform_printf("%s: FAILED to post semaphore %p: %d\n", __func__, &s->sem, rc);
        return -1;
    }

    return 0;
}


int SemaphoreWait(Semaphore* s, int timeout_ms)
{
    if (!s)
    {
        platform_printf("%s: invalid semaphore %p\n", __func__, s);
        return -1;
    }

    wiced_result_t rc = wiced_rtos_get_semaphore(&s->sem, timeout_ms);
    if (rc != WICED_SUCCESS)
    {
        platform_printf("%s: FAILED to post semaphore %p: %d\n", __func__, &s->sem, rc);
        return -1;
    }

    return 0;
}


static void func_wrapper(uint32_t arg)
{
    Thread* t = (Thread*)arg;
    (*t->func)(t->arg);
}


int ThreadCreate(Thread* t, 
        int priority, 
        const char* name, 
        void (*func)(void*), 
        size_t stack_size, 
        void* arg)
{
    if (!t) return -1;
    t->func = func;
    t->arg = arg;
    if (wiced_rtos_create_thread(&t->tid, priority, name, func_wrapper, stack_size, t) != WICED_SUCCESS)
        return -1;

    return 0;
}


int ThreadJoin(Thread* t, int timeout_ms)
{
    if (!t) return -1;
    if (wiced_rtos_thread_join(&t->tid) != WICED_SUCCESS)
        return -1;
    return 0;
}


int ThreadDestroy(Thread* t)
{
    if (!t) return -1;
    if (wiced_rtos_delete_thread(&t->tid) != WICED_SUCCESS)
        return -1;
    return 0;
}

static int stdout_mtx_inited;
static wiced_mutex_t stdout_mtx;

int platform_printf(const char* fmt, ...)
{
    if (!stdout_mtx_inited)
    {
        wiced_rtos_init_mutex(&stdout_mtx);
        stdout_mtx_inited++;
    }

    wiced_rtos_lock_mutex(&stdout_mtx);

    va_list vl;
    va_start(vl, fmt);

    char msg[512];
    unsigned n = vsnprintf(msg, sizeof msg, fmt, vl);
    if (n >= sizeof msg)
        msg[sizeof msg - 1] = '\0';

    int rc = printf("%s", msg);

    va_end(vl);

    wiced_rtos_unlock_mutex(&stdout_mtx);

    return rc;
}


void* platform_malloc(size_t bytes)
{
    return malloc(bytes);
}


void platform_free(void* memory)
{
    free(memory);
}

void platform_sleep(int ms)
{
    wiced_rtos_delay_milliseconds(ms);
}
