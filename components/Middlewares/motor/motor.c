#include "motor.h"
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

char *TAG = "Motor";

void MOTOR_Init(motor_handle_t *motor_handle)
{
    static uint8_t motor_cnt = 0; // 计数这个是第几个电机
    ledc_channel_t ledc_channel;
    if (motor_cnt == 0)
    {
        ledc_channel = LEDC_CHANNEL_0;
    }
    else if (motor_cnt == 1)
    {
        ledc_channel = LEDC_CHANNEL_1;
    }
    else
    {
        ESP_LOGE(TAG, "Motor number error");
        return;
    }
    motor_cnt++;

    gpio_config_t gpio_conf = {
        .pin_bit_mask = 1ULL << motor_handle->gpio_in1,
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&gpio_conf));
    gpio_conf.pin_bit_mask = 1ULL << motor_handle->gpio_in2;
    ESP_ERROR_CHECK(gpio_config(&gpio_conf));

    ledc_config_t motor_ledc_config;
    motor_ledc_config.clk_cfg = LEDC_USE_PLL_DIV_CLK;
    motor_ledc_config.timer_num = LEDC_TIMER_0;
    motor_ledc_config.freq_hz = 1000;
    motor_ledc_config.duty_resolution = LEDC_TIMER_16_BIT;
    motor_ledc_config.channel = ledc_channel;
    motor_ledc_config.gpio_num = motor_handle->gpio_pwmin;
    ledc_init(&motor_ledc_config);
    motor_handle->motor_ledc_config = motor_ledc_config;

    Motor_SetSpeed(motor_handle, 0); // 初始速度为0
}

/**
 * @brief 速度设置
 *
 * @param Speed[-100,100],为正时则为顺时针转，为负时则为逆时针转
 */
void Motor_SetSpeed(motor_handle_t *motor_handle, int8_t Speed)
{
    motor_handle->Speed = Speed;
    if (Speed >= 0) // 顺时针转
    {
        Speed = min(Speed, 100);
        // printf("+,in1:%d,in2:%d,Speed:%d\n", motor_handle->gpio_in1, motor_handle->gpio_in2, Speed);
        gpio_set_level(motor_handle->gpio_in1, 1);
        gpio_set_level(motor_handle->gpio_in2, 0);
        ledc_pwm_set_duty(&(motor_handle->motor_ledc_config), Speed);
        // printf("Speed: %d\n", Speed);
    }
    else if (Speed < 0) // 逆时针转
    {
        Speed = max(Speed, -100);
        // printf("-,in1:%d,in2:%d,Speed:%d\n", motor_handle->gpio_in1, motor_handle->gpio_in2, Speed);
        gpio_set_level(motor_handle->gpio_in1, 0);
        gpio_set_level(motor_handle->gpio_in2, 1);
        ledc_pwm_set_duty(&(motor_handle->motor_ledc_config), -Speed);
        // printf("Speed: %d\n", -Speed);
    }
}