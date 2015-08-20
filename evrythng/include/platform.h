/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 */

/**
 *
 * @file   platform.h
 * @brief  Platform dependent interface.
 *
 **/


#ifndef _PLATFORM_H
#define _PLATFORM_H

#if defined(CONFIG_OS_FREERTOS)

#include <FreeRTOS.h>
#include <task.h>

#include <marvell_api.h>


#define malloc      pvPortMalloc
#define free(ptr)   vPortFree(ptr)

#if !defined(FREERTOS_SIMULATOR)
#include <semphr.h>
#define sleep(sec)  vTaskDelay((sec*1000)/portTICK_RATE_MS);
#define realloc pvPortReAlloc
#define printf wmprintf
#endif

#else

#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>

#endif

#endif //_PLATFORM_H
