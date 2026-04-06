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
#include "adc.h"
#include <stdio.h>

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
// 采集电压并显示
void app_main(void)
{
    esp_err_t ret;
    int adc_data = 0;
    uint16_t voltage = 0;
    float adc_vol = 0;

    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init(); /* LED初始化 */
    // lcd_init(); /* LCD初始化 */
    adc_init(); /* ADC初始化 */

    // lcd_show_string(10, 50, 200, 16, 16, "ESP32-P4", RED);
    // lcd_show_string(10, 70, 200, 16, 16, "ADC TEST", RED);
    // lcd_show_string(10, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    // lcd_show_string(10, 130, 200, 16, 16, "ADC_VOL:0.000V", BLUE);

    while (1)
    {
        voltage = adc_get_result_voltage(5); /* 获取平均电压数值 */
        adc_vol = (float)voltage / 1000;
        adc_data = (int)adc_vol;            /* 获取整数部分 */
        printf("ADC_VOL:%.3fV\n", adc_vol); /* 显示电压值 */
        // lcd_show_xnum(74, 130, adc_data, 1, 16, 0, BLUE); /* 显示电压值的整数部分 */

        // adc_vol -= adc_data;
        // adc_vol *= 1000;        /* 小数部分放大1000倍方便显示 */
        // adc_vol = (int)adc_vol; /* 取整 */
        // lcd_show_xnum(90, 130, adc_vol, 3, 16, 0x80, BLUE); /* 显示电压值的小数部分 */

        LED_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
