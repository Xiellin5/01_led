#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "led.h"
#include "esptimer.h"
void app_main(void)
{
    led_init();
    esptimer_init(1000000);
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
