#if !defined(__MQTT_PLATFORM_)
#define __MQTT_PLATFORM_

#if defined(POSIX_PLATFORM)
    #include "POSIX/types.h"
#elif defined(WICED_PLATFORM)
    #include "WICED/types.h"
#elif defined(MARVELL_PLATFORM)
    #include "marvell/types.h"
#endif

/* all failure return codes must be negative */
enum returnCode 
{ 
    MQTT_CONNECTION_LOST = -3, 
    MQTT_BUFFER_OVERFLOW = -2, 
    MQTT_FAILURE = -1, 
    MQTT_SUCCESS = 0 
};

void TimerInit(Timer*);
void TimerDeinit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
int  TimerLeftMS(Timer*);

void NetworkInit(Network*);
void NetworkSecuredInit(Network*, const char* ca_buf, size_t ca_size);
int  NetworkConnect(Network*, char*, int);
void NetworkDisconnect(Network*);
int  NetworkRead(Network*, unsigned char*, int, int);
int  NetworkWrite(Network*, unsigned char*, int, int);

void MutexInit(Mutex*);
void MutexDeinit(Mutex*);
int  MutexLock(Mutex*);
int  MutexUnlock(Mutex*);

void SemaphoreInit(Semaphore*);
void SemaphoreDeinit(Semaphore*);
int SemaphorePost(Semaphore*);
int SemaphoreWait(Semaphore*, int);

int ThreadCreate(Thread* thread, 
        int priority, 
        const char* name, 
        void (*func)(void*), 
        size_t stack_size, 
        void* arg);
int ThreadJoin(Thread*, int);
int ThreadDestroy(Thread*);

int platform_printf(const char* fmt, ...);

void* platform_malloc(size_t bytes);
void  platform_free(void* memory);

void platform_sleep(int ms);

#endif //__MQTT_PLATFORM_
