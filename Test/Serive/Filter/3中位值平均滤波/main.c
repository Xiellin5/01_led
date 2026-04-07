#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "led.h"
#include "filter.h"
#include <stdio.h>

// 测试用原始数据（模拟带脉冲干扰的采样值）
#define DATA_LENGTH 10
static const uint16_t raw_data[DATA_LENGTH] = {100, 180, 102, 105, 98, 200, 101, 99, 103, 104};

void print(uint8_t *str)
{
    printf((char *)str);
}

/**
 * @brief       程序入口
 * @param       无
 * @retval      无
 */
// 滤波测试
void app_main(void)
{
    esp_err_t ret;
    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
#define QUEUE_LEN 3
    printf("===== 3. 中位值平均滤波测试 =====\r\n");
    // 取索引0的5个值（100,180,102,105,98），去最大180、最小98，平均(100+102+105)/3=102
    uint16_t median_avg_res = median_average_filter(raw_data, DATA_LENGTH, 0, 5);
    printf("索引0取5个值中位值平均：%d（理论值102）\r\n", median_avg_res);

    printf("===== 4. 限幅平均滤波测试 =====\r\n");
    uint16_t limit_queue[QUEUE_LEN] = {100, 102, 102}; // 初始化队列
    uint16_t limit = 51;                               // 限幅阈值
    // 测试第3个值180（超阈值，限幅为51，再滑动平均）
    uint16_t limit_avg_res = amplitude_limit_average_filter(limit_queue, QUEUE_LEN, 180, limit);
    printf("限幅阈值51，新值180的限幅平均结果：%d（理论值(102+102+102)/3=102）\r\n", limit_avg_res);

    printf("===== 5. 一阶滞后滤波测试 =====\r\n");
    uint16_t last_lag_res = 100; // 初始值
    float alpha = 0.1f;          // 滞后系数（越小越灵敏）
    for (uint16_t i = 0; i < 5; i++)
    {
        last_lag_res = first_order_lag_filter(raw_data[i], last_lag_res, alpha);
        printf("第%d次一阶滞后结果（α=0.1）：%d\r\n", i + 1, last_lag_res);
    }
    led_init();
    while (1)
    {
        LED_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
