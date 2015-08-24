/*
 * (c) Copyright 2012 EVRYTHNG Ltd London / Zurich
 * www.evrythng.com
 */

#include "tests.h"
#include "evrythng_platform.h"

int main(void)
{
#if defined(CONFIG_OS_FREERTOS)
    xTaskCreate(RunAllTests, "tests", 1024, 0, 0, NULL);
    vTaskStartScheduler();
#else
	RunAllTests(0);
#endif

    return 0;
}
