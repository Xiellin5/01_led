#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"
// 测试小车代码

int gpio_num[] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 32, 33,
    23, 51, 49, 50, 48, 47, 46, 45, 36, 35, 30, 26, 27, 28, 29, 31, 25, 24, 52, 53, 0};
char *TAG = "main";
// 现象：2~15的引脚电压最大为1.24V，其他可以达到3.3V
// gnd测试全部通过
void vcc_test(void)
{
    int level = 1;
    for (int i = 0; gpio_num[i] != 0; i++)
    {
        gpio_config_t gpio_conf = {
            .pin_bit_mask = 1ULL << gpio_num[i],
            .mode = GPIO_MODE_INPUT_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE};
        ESP_ERROR_CHECK(gpio_config(&gpio_conf));
        gpio_set_level(gpio_num[i], level);
        printf("gpio set level:%d\n", level);
        vTaskDelay(pdMS_TO_TICKS(100));
        printf("gpio read level:%d\n", gpio_get_level(gpio_num[i]));
        if (gpio_get_level(gpio_num[i]) != level)
        {
            ESP_LOGE(TAG, "gpio:%d test fail", gpio_num[i]);
        }
        else
        {
            ESP_LOGI(TAG, "gpio:%d test pass", gpio_num[i]);
        }
    }
}

void gnd_test(void)
{
    int level = 0;
    for (int i = 0; gpio_num[i] != 0; i++)
    {
        gpio_config_t gpio_conf = {
            .pin_bit_mask = 1ULL << gpio_num[i],
            .mode = GPIO_MODE_INPUT_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE};
        ESP_ERROR_CHECK(gpio_config(&gpio_conf));
        gpio_set_level(gpio_num[i], level);
        printf("gpio set level:%d\n", level);
        vTaskDelay(pdMS_TO_TICKS(100));
        printf("gpio read level:%d\n", gpio_get_level(gpio_num[i]));
        if (gpio_get_level(gpio_num[i]) != level)
        {
            ESP_LOGE(TAG, "gpio:%d test fail", gpio_num[i]);
        }
        else
        {
            ESP_LOGI(TAG, "gpio:%d test pass", gpio_num[i]);
        }
    }
}

void app_main(void)
{
    // vcc_test();
    // vTaskDelay(pdMS_TO_TICKS(1000));
    gnd_test();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
