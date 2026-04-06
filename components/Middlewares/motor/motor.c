#include "motor.h"
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

static ledc_config_t motor_ledc_config;
void Motor_SetSpeed(int8_t Speed);
void MOTOR_Init(void)
{
    gpio_config_t gpio_conf = {
        .pin_bit_mask = 1ULL << MOTOR_AIN1_PIN,
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&gpio_conf));
    gpio_conf.pin_bit_mask = 1ULL << MOTOR_AIN2_PIN;
    ESP_ERROR_CHECK(gpio_config(&gpio_conf));

    motor_ledc_config.clk_cfg = LEDC_USE_PLL_DIV_CLK;
    motor_ledc_config.timer_num = LEDC_TIMER_0;
    motor_ledc_config.freq_hz = 1000;
    motor_ledc_config.duty_resolution = LEDC_TIMER_16_BIT;
    motor_ledc_config.channel = LEDC_CHANNEL_0;
    motor_ledc_config.gpio_num = MOTOR_PWMA_PIN;
    ledc_init(&motor_ledc_config);

    Motor_SetSpeed(0); // 初始速度为0
}

/**
 * @brief 速度设置
 *
 * @param Speed[-100,100],为正时则为顺时针转，为负时则为逆时针转
 */
void Motor_SetSpeed(int8_t Speed)
{
    if (Speed >= 0) // 顺时针转
    {
        Speed = min(Speed, 100);
        gpio_set_level(MOTOR_AIN1_PIN, 1);
        gpio_set_level(MOTOR_AIN2_PIN, 0);
        ledc_pwm_set_duty(&motor_ledc_config, Speed);
        // printf("Speed: %d\n", Speed);
    }
    else if (Speed < 0) // 逆时针转
    {
        Speed = max(Speed, -100);
        gpio_set_level(MOTOR_AIN1_PIN, 0);
        gpio_set_level(MOTOR_AIN2_PIN, 1);
        ledc_pwm_set_duty(&motor_ledc_config, -Speed);
        // printf("Speed: %d\n", -Speed);
    }
}