/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 */

#if !defined(__MQTT_PLATFORM_)
#define __MQTT_PLATFORM_

#include "platform_types.h"

/* all failure return codes must be negative */
enum returnCode 
{ 
    MQTT_CONNECTION_LOST = -3, 
    MQTT_BUFFER_OVERFLOW = -2, 
    MQTT_FAILURE = -1, 
    MQTT_SUCCESS = 0 
};

void platform_timer_init(Timer*);
void platform_timer_deinit(Timer*);
char platform_timer_isexpired(Timer*);
void platform_timer_countdown(Timer*, unsigned int);
int  platform_timer_left(Timer*);

void platform_network_init(Network*);
void platform_network_securedinit(Network*, const char* ca_buf, size_t ca_size);
int  platform_network_connect(Network*, char*, int);
void platform_network_disconnect(Network*);
int  platform_network_read(Network*, unsigned char*, int, int);
int  platform_network_write(Network*, unsigned char*, int, int);

void platform_mutex_init(Mutex*);
void platform_mutex_deinit(Mutex*);
int  platform_mutex_lock(Mutex*);
int  platform_mutex_unlock(Mutex*);

void platform_semaphore_init(Semaphore*);
void platform_semaphore_deinit(Semaphore*);
int platform_semaphore_post(Semaphore*);
int platform_semaphore_wait(Semaphore*, int);

int platform_thread_create(Thread* thread, 
        int priority, 
        const char* name, 
        void (*func)(void*), 
        size_t stack_size, 
        void* arg);
int platform_thread_join(Thread*, int);
int platform_thread_destroy(Thread*);

int platform_printf(const char* fmt, ...);

void* platform_malloc(size_t bytes);
void* platform_realloc(void* ptr, size_t bytes);
void  platform_free(void* memory);

void platform_sleep(int ms);

#endif //__MQTT_PLATFORM_
