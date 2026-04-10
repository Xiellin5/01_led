#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "led.h"
#include "car.h"

// 直行
void car_forward_test(void)
{
    car_set_motor_l(50);
    car_set_motor_r(50);
}
// 后退
void car_back_test(void)
{
    car_set_motor_l(-50);
    car_set_motor_r(-50);
}
// 左转
void car_left_test(void)
{
    car_set_motor_l(30);
    car_set_motor_r(60);
}
// 右转
void car_right_test(void)
{
    car_set_motor_l(60);
    car_set_motor_r(30);
}
static uint8_t i = 0;
// 测试小车综合运动代码
void app_main(void)
{
    car_Init();
    led_init();
    car_forward_test();

    while (1)
    {
        LED_TOGGLE();
        if (i == 0)
        {
            car_forward_test();
        }
        else if (i == 1)
        {
            car_back_test();
        }
        else if (i == 2)
        {
            car_left_test();
        }
        else if (i == 3)
        {
            car_right_test();
        }
        i++;
        if (i == 4)
        {
            i = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1000 * 3));
    }
}
