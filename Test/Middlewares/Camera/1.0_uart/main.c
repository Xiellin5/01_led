#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart.h"
#include "camera_uart.h"

void app_main(void)
{
    printf("=== System Booting ===\n");

    // 1. 初始化底层硬件
    UART_Init();
    
    // 2. 初始化摄像头业务层并开启接收任务
    Camera_Init();
    
    printf("=== OpenMV Image Receiver Started ===\n");

    // 3. 主循环
    while (1)
    {
        // 判断是否收到了完整的一帧 JPEG 图片
        if (Camera_Is_Ready())
        {
            // 获取图片结构体指针
            Camera_Image_t* img = Camera_Get_Image();
            
            printf("🎉 主函数收到图片! 长度 = %" PRIu32 " 字节\n", img->length);
            
            // ==========================================
            // 👉 在这里执行你的解压缩(esp_jpeg)、LCD显示 或 AI模型运算
            // 比如: esp_jpeg_decode(img->data, img->length, ...);
            // ==========================================

            // 处理完毕后，必须清除标志位，允许后台任务继续写入新图片
            Camera_Clear_Flag();
        }

        // 稍微延时，防止主任务占用100% CPU 触发看门狗复位
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}