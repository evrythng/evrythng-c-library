/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 */

#if !defined(_MQTT_POSIX_)
#define _MQTT_POSIX_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct Timer
{
	struct timeval end_time;
} Timer;

typedef struct Network
{
	int my_socket;
} Network;

typedef struct Mutex
{
	pthread_mutex_t mtx;
} Mutex;

typedef struct Semaphore
{
    sem_t sem;
} Semaphore;

typedef struct Thread
{
    pthread_t tid;
    void* arg;
    void (*func)(void*);
} Thread;

#endif //_MQTT_POSIX_
