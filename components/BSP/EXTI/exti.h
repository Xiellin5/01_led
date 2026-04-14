#pragma once
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_rom_sys.h"
#include "led.h"
#define BOOT_INT_GPIO_PIN GPIO_NUM_35

#define BOOT_INT gpio_get_level(BOOT_INT_GPIO_PIN)

void exti_init(void);
