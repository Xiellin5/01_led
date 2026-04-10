#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "ledc.h"
#include "motor.h"

// 电机从0加速到全速，再从全速减为0
static int dir = 1;
static int ledpwmval = 0;
void app_main(void)
{
    MOTOR_Init();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (dir == 1)
        {
            ledpwmval += 5;
        }
        else
        {
            ledpwmval -= 5;
        }

        if (ledpwmval > 95)
        {
            dir = 0;
        }
        if (ledpwmval < 5)
        {
            dir = 1;
        }
        Motor_SetSpeed(ledpwmval);
    }
}
