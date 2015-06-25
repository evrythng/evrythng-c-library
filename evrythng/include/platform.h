/**
 *
 * @file   platform.h
 * @brief  Platform dependent interface.
 *
 **/


#ifndef _PLATFORM_H
#define _PLATFORM_H

#if defined(CONFIG_OS_FREERTOS)

#include "FreeRTOS.h"
#include "task.h"
#define malloc      pvPortMalloc
#define free        vPortFree
#define sleep(sec)  vTaskDelay((sec*1000)/portTICK_RATE_MS);
#define printf(...)

#else

#include <unistd.h>
#include <stdio.h>

#endif

#endif //_PLATFORM_H
