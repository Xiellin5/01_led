/**
 ******************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       ADC 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ******************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32-P4 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 ******************************************************************************
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "led.h"
#include "esp8266.h"
#include <stdio.h>

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
// mcu通过串口1（esp8266的串口）发送AT给PC，PC端选择性发送OK来回应，
// 如果回应则mcu的监控端出现success，否则fail句子
void app_main(void)
{
    esp_err_t ret;

    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init();
    ESP8266_Init();
    // ESP8266_Send_test();
    // ESP8266_AT_Test();
    while (1)
    {
        // ESP8266_SendString((uint8_t *)"Hello, world!\r\n");
        ret = ESP8266_Send_Cmd((uint8_t *)"AT\r\n", (uint8_t *)"OK", 100);
        if (ret == ESP_OK)
        {
            printf("AT command send success!\r\n");
        }
        else
        {
            printf("AT command send failed!\r\n");
        }
        LED_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
