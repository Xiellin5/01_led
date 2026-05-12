#include <stdio.h>
#include "espdet_run.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "car_system.h"
#include "my_queue.h"
extern const uint8_t person_jpg_start[] asm("_binary_person_jpg_start");
extern const uint8_t person_jpg_end[] asm("_binary_person_jpg_end");

// 加入ai模块的测试
// 现象:发送flash中的图片首尾地址后，执行ai任务，并且打印结果
void app_main(void)
{
    if (xTaskCreate(system_run, "system", 1024 * 2 + 512, NULL, 1, NULL) == pdPASS)
    {
        printf("system task created\n");
    };
    vTaskDelay(pdMS_TO_TICKS(100));
    system_send_rst();
    printf("send system reset\n");
    while (1)
    {
        jpeg_queue_send(person_jpg_start, person_jpg_end);
        printf("send person jpg\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}