#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "led.h"
#include "filter.h"
#include <stdio.h>

// 测试用原始数据（模拟带干扰的采样值）
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
// 中位值滤波
void app_main(void)
{
    esp_err_t ret;
    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    printf("===== 算术平均滤波测试 =====\n");
    // 测试1：从索引0开始，取5个值（100,180,102,105,98）做算术平均
    uint16_t avg_result1 = arithmetic_average_filter(raw_data, DATA_LENGTH, 0, 5);
    printf("从索引0取5个值的算术平均值：%d（理论值：(100+180+102+105+98)/5=117）\n", avg_result1);

    // 测试2：从索引2开始，取3个值（102,105,98）做算术平均
    uint16_t avg_result2 = arithmetic_average_filter(raw_data, DATA_LENGTH, 2, 3);
    printf("从索引2取3个值的算术平均值：%d（理论值：(102+105+98)/3=101）\n\n", avg_result2);

    printf("===== 递推平均（滑动平均）滤波测试 =====\n");
// 初始化滑动队列（长度3，初始值设为0）
#define QUEUE_LENGTH 3
    uint16_t slide_queue[QUEUE_LENGTH] = {0};

    // 遍历原始数据，逐次进行滑动平均滤波
    for (uint16_t i = 0; i < DATA_LENGTH; i++)
    {
        uint16_t slide_result = recursive_average_filter(slide_queue, QUEUE_LENGTH, raw_data[i]);
        printf("第%d次采样值：%d，滑动平均值：%d\n", i + 1, raw_data[i], slide_result);
    }

    /* 输出结果说明：
       第1次：队列[0,0,100] → 平均值 33
       第2次：队列[0,100,180] → 平均值 93
       第3次：队列[100,180,102] → 平均值 127
       第4次：队列[180,102,105] → 平均值 129
       第5次：队列[102,105,98] → 平均值 101
       ... 后续依次滑动，体现“先进先出”的特点
    */
    led_init();
    while (1)
    {
        LED_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}