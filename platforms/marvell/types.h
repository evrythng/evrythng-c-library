/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(_MQTT_MARVELL_)
#define _MQTT_MARVELL_

#include <stddef.h>
#include <wm_os.h>

#include "FreeRTOS.h"
#include "task.h"

typedef struct Timer
{
	portTickType xTicksToWait;
	xTimeOutType xTimeOut;
} Timer;

typedef struct Network
{
    int socket;
} Network;

typedef struct Mutex
{
    os_mutex_t mutex;
} Mutex;

typedef struct Semaphore
{
    os_semaphore_t sem;
} Semaphore;

typedef struct Thread
{
    os_thread_t tid;
    void* arg;
    void (*func)(void*);
    Semaphore join_sem;
} Thread;

#endif //_MQTT_MARVELL_
