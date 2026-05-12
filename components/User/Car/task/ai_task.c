#include "ai_task.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"
#include "esp_log.h"

#define ai_run(jpg_start, jpg_end) (run_human_fall_detect(jpg_start, jpg_end, ai_task_callback, NULL))
static char *TAG = "ai_task";

static void ai_task_callback(const ai_result_t *result, void *user_data)
{
    // 处理每个检测结果，例如保存到数组、触发GPIO等
    printf("Fall detected: category: %d,score=%.2f box=[%d,%d,%d,%d]\n",
           result->category, result->score, result->box[0], result->box[1], result->box[2], result->box[3]);
    ai_result_queue_send(*result);
}

void ai_task(void *pvParameters)
{
    const uint8_t *start = NULL;
    const uint8_t *end = NULL;
    while (1)
    {
        UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "watermark of system_run task = %u\n", watermark);
        if (jpeg_queue_receive(&start, &end, portMAX_DELAY))
        {
            printf("ai_task received jpeg image\n");
            ai_run(start, end);
        }
    }
}
