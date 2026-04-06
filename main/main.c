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
void app_main(void)
{
    esp_err_t ret;
    char *TAG = "MAIN";
    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init();
    ESP8266_Init();

    ret = ESP8266_UDP_PS(10);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "UDP PS build failed!");
    }
    ESP_LOGI(TAG, "UDP PS build success!");
    printf("本地esp8266的ip相关信息:%s\n", (char *)wifi.wifi_ip);

    uint8_t data[128];
    uint16_t length = 0;
    while (1)
    {
        ret = ESP8266_Start_CIPSend();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Start CIPSEND failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Start CIPSEND success!");
        }

        ESP8266_SendString((uint8_t *)"mcu:Hello,world!\r\n");
        ESP8266_ReadString(data, &length);
        if (length == 0)
        {
            ESP_LOGE(TAG, "No data from pc!");
        }
        else
        {
            ESP_LOGI(TAG, "Data from pc:%s", data);
        }

        vTaskDelay(pdMS_TO_TICKS(100));

        ret = ESP8266_Stop_CIPSend();
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Stop CIPSEND failed!");
        }
        else
        {
            ESP_LOGI(TAG, "Stop CIPSEND success!");
        }
        LED_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
