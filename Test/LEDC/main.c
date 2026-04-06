#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "ledc.h"

// IO51呼吸灯
static int dir = 1;
static int ledpwmval = 0;
void app_main(void)
{
    ledc_config_t ledc_config = {
        .clk_cfg = LEDC_USE_PLL_DIV_CLK,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .duty_resolution = LEDC_TIMER_16_BIT,
        .channel = LEDC_CHANNEL_0,
        .gpio_num = LEDC_PWM_CH0_GPIO,
    };
    ledc_init(&ledc_config);
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
        ledc_pwm_set_duty(&ledc_config, ledpwmval);
    }
}
