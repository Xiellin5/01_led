#pragma once

#include "esp_system.h"
#include "driver/ledc.h"

typedef struct ledc_config_t ledc_config_t;
struct ledc_config_t
{
    // 定时器配置
    ledc_clk_cfg_t clk_cfg;
    ledc_timer_t timer_num;
    uint32_t freq_hz;
    ledc_timer_bit_t duty_resolution;

    // 通道配置
    ledc_channel_t channel;
    uint32_t duty;
    int gpio_num;
};

#define LEDC_PWM_CH0_GPIO 51
void ledc_init(ledc_config_t *ledc_config);
uint32_t ledc_duty_pow(uint32_t duty, uint8_t m, uint8_t n);
void ledc_pwm_set_duty(ledc_config_t *ledc_config, uint16_t duty);
