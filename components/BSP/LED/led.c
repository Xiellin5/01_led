#include "led.h"

void led_init(void)
{
    gpio_config_t gpio_conf = {
        .pin_bit_mask = 1ULL << LED_GPIO_PIN,
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&gpio_conf));

    LED_OFF();
}

uint32_t led_get_level(void)
{
    return gpio_get_level(LED_GPIO_PIN);
}
