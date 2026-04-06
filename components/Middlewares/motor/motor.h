#ifndef __MOTOR_H__
#define __MOTOR_H__
#include "stdint.h"
#include "driver/gpio.h"
#include "ledc.h"

#define MOTOR_AIN1_PIN 28
#define MOTOR_AIN2_PIN 29
#define MOTOR_PWMA_PIN 31

void MOTOR_Init(void);
void Motor_SetSpeed(int8_t Speed);

#endif /* __MOTOR_H__ */
