#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "exti.h"
#include "led.h"
#include "nvs.h"
#include "nvs_bootloader.h"

// 按下BOOT，灯TOGGLE
// 中断书写注意事项：1.中断处理函数不要进行前向声明，推荐直接放在注册的前面
// 中断函数里面不要写printf之类的打印
void app_main(void)
{
    exti_init();
    led_init();
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
