#include <stdio.h>
#include "car_system.h"
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"

// 系统模块测试
// 效果:system创建后过了一段事件被reset,然后继续start和work
void app_main(void)
{
    if (xTaskCreate(system_run, "system", 1024, NULL, 1, NULL) == pdPASS)
    {
        printf("system task created\n");
    };
    vTaskDelay(pdMS_TO_TICKS(100));
    system_send_rst();
    printf("send system reset\n");
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}