#if !defined(__MQTT_PLATFORM_)
#define __MQTT_PLATFORM_

#if defined(POSIX_PLATFORM)
    #include "MQTTLinux.h"
#elif defined(WICED_PLATFROM)
    #include ".h"
#elif defined(MARVELL_PLATFROM)
    #include ".h"
#endif

/* all failure return codes must be negative */
enum returnCode { CONNECTION_LOST = -3, BUFFER_OVERFLOW = -2, FAILURE = -1, SUCCESS = 0 };

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int  TimerLeftMS(Timer*);

void NetworkInit(Network*);
int  NetworkConnect(Network*, char*, int);
void NetworkDisconnect(Network*);
int  NetworkRead(Network*, unsigned char*, int, int);
int  NetworkWrite(Network*, unsigned char*, int, int);

void MutexInit(Mutex*);
int  MutexLock(Mutex*);
int  MutexUnlock(Mutex*);


#endif
