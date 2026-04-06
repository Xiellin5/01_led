#include "unity.h"
#include "serial.h"
#include "protocol.h"
#include "FreeRTOS/FreeRTOS.h"

// gcode命令接收测试
// 输入任意前缀字符，加上正式的Gcode命令，以换行符结尾，会自动读取，而后打印，字母会转换为大写
void app_main(void)
{
    // 初始化串口（由你提供的 serial.c 实现）
    serial_init();
    uint8_t c;
    while (1)
    {
        c = serial_read();       // 读取串口数据
        if (c != SERIAL_NO_DATA) // 表示开始读取数据
        {
            printf("c = %c\n", c);
            protocol_main_loop();
        }
        else
        {
            printf("no data\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    return;
}