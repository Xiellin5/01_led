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
// 板子时序测试，效果为输出正常的时序
void app_main(void)
{
    car_Init();
    led_init();
    car_right_test();

    while (1)
    {
        LED_TOGGLE();
        int Ain1 = gpio_get_level(CAR_L_AIN1_PIN);
        int Ain2 = gpio_get_level(CAR_L_AIN2_PIN);
        int Bin1 = gpio_get_level(CAR_R_BIN1_PIN);
        int Bin2 = gpio_get_level(CAR_R_BIN2_PIN);
        printf("Ain1:%d,Ain2:%d,Bin1:%d,Bin2:%d\n", Ain1, Ain2, Bin1, Bin2);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
