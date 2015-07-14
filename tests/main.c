#include "tests.h"
#include "platform.h"

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
