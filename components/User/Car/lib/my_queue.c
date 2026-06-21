#include "my_queue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

/*----------------------------------------队列定义区----------------------------------------*/
static QueueHandle_t camera_jpg_queue = NULL;
static QueueHandle_t ai_result_queue = NULL;

/*-----------------------------------------------------------------------------------------*/
static const char *TAG_AI = "ai_result_queue";

/**
 * @brief 初始化JPEG数据队列（最多缓存N帧）
 *
 * @param if_rst 如果已经存在队列，是否重置队列
 */
void jpeg_queue_init(bool if_rst)
{
    if (camera_jpg_queue == NULL)
    {
        camera_jpg_queue = xQueueCreate(CAMERA_JPEG_QUEUE_SIZE, sizeof(jpeg_msg_t));
    }
    if (if_rst)
    {
        xQueueReset(camera_jpg_queue);
    }
}

/**
 * @brief 发送JPEG数据到队列（生产者调用）
 * @param start 数据起始指针
 * @param end   数据结束指针
 * @return true成功，false失败
 */
// 修改 my_queue.c
bool jpeg_queue_send(const uint8_t *start, const uint8_t *end, TickType_t timeout)
{
    if (camera_jpg_queue == NULL)
    {
        jpeg_queue_init(false);
    }
    jpeg_msg_t msg = {.start = start, .end = end};
    // 将原先的 0 改为 timeout 参数
    return xQueueSend(camera_jpg_queue, &msg, timeout) == pdTRUE;
}

/**
 * @brief 接收JPEG数据（消费者调用，可阻塞）
 * @param start 输出起始指针
 * @param end   输出结束指针
 * @param timeout 等待时间（portMAX_DELAY 表示永久等待）
 * @return true成功，false超时或失败
 */
bool jpeg_queue_receive(const uint8_t **start, const uint8_t **end, TickType_t timeout)
{
    if (camera_jpg_queue == NULL)
    {
        jpeg_queue_init(false);
    }
    jpeg_msg_t msg;
    if (xQueueReceive(camera_jpg_queue, &msg, timeout) == pdTRUE)
    {
        *start = msg.start;
        *end = msg.end;
        return true;
    }
    return false;
}

void ai_result_queue_init(bool if_rst)
{
    if (ai_result_queue == NULL)
    {
        ai_result_queue = xQueueCreate(AI_RESULT_QUEUE_SIZE, sizeof(ai_result_t));
        if (ai_result_queue == NULL)
        {
            ESP_LOGE(TAG_AI, "Failed to create AI result queue");
        }
        else
        {
            ESP_LOGI(TAG_AI, "AI result queue created");
        }
    }
    if (if_rst)
    {
        xQueueReset(ai_result_queue);
    }
}

bool ai_result_queue_send(ai_result_t result)
{
    if (ai_result_queue == NULL)
    {
        ai_result_queue_init(false);
    }
    return xQueueSend(ai_result_queue, &result, 0) == pdTRUE;
}

bool ai_result_queue_receive(ai_result_t *result, TickType_t timeout)
{
    if (ai_result_queue == NULL)
    {
        ai_result_queue_init(false);
    }
    return xQueueReceive(ai_result_queue, result, timeout) == pdTRUE;
}