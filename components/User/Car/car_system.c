#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "car_system.h"
#include "my_queue.h"   // 确保引入了消息队列的头文件
/*---------------------------------- 外部函数与任务声明 -----------------------------------*/
extern void camera_task(void *pvParameters);
extern void test_consumer_task(void *pvParameters);
extern void camera_task_memory_cleanup(void);

/*-----------------------------------------任务句柄----------------------------------------*/
static TaskHandle_t ai_task_handle = NULL;
static TaskHandle_t camera_task_handle = NULL; // <-- 增加这一行
static TaskHandle_t test_consumer_handle = NULL; // 临时测试接收任务句柄

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
        //ESP_LOGI(TAG, "watermark of system_run task = %u\n", watermark);
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
            //ESP_LOGI(TAG, "system work\n");
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

    // 1. 创建 AI 任务 (优先级 19)
    if (ai_task_handle == NULL) {
        if (xTaskCreate(ai_task, "ai_task", 1024 * 8, NULL, AI_TASK_PRIORITY, &ai_task_handle) != pdPASS) {
            ESP_LOGE(TAG, "create ai_task failed");
        } else {
            ESP_LOGI(TAG, "create ai_task success");
        }
    }

    // 3. 创建 Camera 任务 
    if (camera_task_handle == NULL) {
        // 分配 12KB 栈空间，优先级使用 car_setting.h 中的定义
        if (xTaskCreate(camera_task, "camera_task", 1024 * 12, NULL, CAMERA_TASK_PRIORITY, &camera_task_handle) != pdPASS) {
            ESP_LOGE(TAG, "create camera_task failed");
        } else {
            ESP_LOGI(TAG, "create camera_task success");
        }
    }
}


/**
 * @brief 删除任务
 *
 */
void system_delete(void)
{
    if (ai_task_handle != NULL) {
        vTaskDelete(ai_task_handle);
        ai_task_handle = NULL;
    }
    
    // 2. 删除相机任务
    if (camera_task_handle != NULL) {
        vTaskDelete(camera_task_handle);
        camera_task_handle = NULL;
        ESP_LOGI(TAG, "delete camera_task success");
    }

    // 3. 释放相机任务中申请的 DMA 内存，彻底杜绝复位时的内存泄漏
    camera_task_memory_cleanup();
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
