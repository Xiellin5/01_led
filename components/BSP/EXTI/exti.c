#include "exti.h"

static void IRAM_ATTR exti_gpio_isr_handler_t(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    if (gpio_num == BOOT_INT_GPIO_PIN)
    {
        esp_rom_delay_us(20000);
        if (BOOT_INT == 0)
        {
            LED_TOGGLE();
        }
    }
}

void exti_init(void)
{
    gpio_config_t gpio_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << BOOT_INT_GPIO_PIN,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BOOT_INT_GPIO_PIN,
                                         exti_gpio_isr_handler_t,
                                         (void *)BOOT_INT_GPIO_PIN));
}
