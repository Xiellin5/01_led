#pragma once
#include "driver/gpio.h"
#include "esp_check.h"
#define LED_GPIO_PIN 51

#define LED_ON()                         \
    {                                    \
        gpio_set_level(LED_GPIO_PIN, 0); \
    }

#define LED_OFF()                        \
    {                                    \
        gpio_set_level(LED_GPIO_PIN, 1); \
    }

#define LED_TOGGLE()                                                 \
    {                                                                \
        gpio_set_level(LED_GPIO_PIN, !gpio_get_level(LED_GPIO_PIN)); \
    }

void led_init(void);
uint32_t led_get_level(void);
