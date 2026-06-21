#include "car_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    // 启动系统管理任务
    // 它会进入 system_run -> system_create() -> 创建 camera 和 ai 任务
    xTaskCreate(system_run, "system_run", 1024 * 4, NULL, 10, NULL);
}