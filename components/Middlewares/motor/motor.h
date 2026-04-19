#pragma once
#include "stdint.h"
#include "driver/gpio.h"
#include "ledc.h"
#include "esp_log.h"

#define MOTOR_AIN1_PIN 28
#define MOTOR_AIN2_PIN 29
#define MOTOR_PWMA_PIN 31

#define MOTOR_BIN1_PIN 16
#define MOTOR_BIN2_PIN 17
#define MOTOR_PWMB_PIN 18

typedef struct motor_handle_t motor_handle_t;
struct motor_handle_t
{
    ledc_config_t motor_ledc_config;

    uint8_t gpio_in1;
    uint8_t gpio_in2;
    uint8_t gpio_pwmin;

    int8_t Speed;
};

void MOTOR_Init(motor_handle_t *motor_handle);
void Motor_SetSpeed(motor_handle_t *motor_handle, int8_t Speed);
