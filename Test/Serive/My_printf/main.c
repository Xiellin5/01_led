#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "led.h"
#include "esp8266.h"
#include "My_printf.h"
#include <stdio.h>
#include <math.h>

void print(uint8_t *str)
{
    printf((char *)str);
}

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
// 效果：以firewater格式打印sin和cos
void app_main(void)
{
    esp_err_t ret;
    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    print_register(ESP8266_Init, print);
    led_init();
    float i = 0;
    while (1)
    {
        print_FireWater(2, sin(i), cos(i));
        i += 0.1;
        LED_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
