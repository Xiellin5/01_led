#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "led.h"
#include "pid.h"
#include "My_printf.h"
#include "esp8266.h"
#include <stdio.h>

void print(uint8_t *str)
{
    printf((char *)str);
}

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
// pid控制，输出目标当前位置(VOFA+可看到波形图)
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
    pid_t pid;
    pid_create(&pid, 0);
    pid_init(&pid);
    pid_set_k(&pid, 0.80, 0.01, 0.001);
    pid_set_target(&pid, 100);
    pid_set_current(&pid, 0);
    while (1)
    {
        LED_TOGGLE();
        print_FireWater(1, pid.current);
        pid_calc(&pid);
        pid_set_current(&pid, pid.current + pid.total_out);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
