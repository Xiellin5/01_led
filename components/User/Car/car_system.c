#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "car_system.h"
/*-----------------------------------------任务句柄----------------------------------------*/
static TaskHandle_t ai_task_handle = NULL;

/*---------------------------------------------------------------------------------------- */

#define S_idle 0
#define S_work 1
static SemaphoreHandle_t rst = NULL;
static int sys_state = S_idle;
static char *TAG = "system";

/**
 * @brief 主循环
 *
 */
void system_run(void *pvParameters)
{
    sys_state = S_idle;
    rst = xSemaphoreCreateBinary();
    while (1)
    {
        UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "watermark of system_run task = %u\n", watermark);
        if (sys_state == S_idle)
        {
            system_create();
            sys_state = S_work;
            ESP_LOGI(TAG, "system start\n");
        }
        else if (sys_state == S_work)
        {
            if (xSemaphoreTake(rst, 0) == pdTRUE)
            {
                system_delete();
                sys_state = S_idle;
                ESP_LOGI(TAG, "system reset\n");
            }
            ESP_LOGI(TAG, "system work\n");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
/**
 * @brief 创建任务
 *
 */
void system_create(void)
{
    // 通信队列初始化
    jpeg_queue_init(true);
    ai_result_queue_init(true);
    if (ai_task_handle == NULL)
    {
        if (xTaskCreate(ai_task, "ai_task", 1024 * 4, NULL, AI_TASK_PRIORITY, &ai_task_handle) != pdPASS)
        {
            ESP_LOGE(TAG, "create ai_task failed");
        }
        else
        {
            ESP_LOGI(TAG, "create ai_task success");
        };
    }
}

/**
 * @brief 删除任务
 *
 */
void system_delete(void)
{
    if (ai_task_handle != NULL)
    {
        vTaskDelete(ai_task_handle);
        ai_task_handle = NULL;
    }
}

/**
 * @brief 软件复位
 *
 */
void system_send_rst(void)
{
    if (rst != NULL)
    {
        xSemaphoreGive(rst);
    }
}
