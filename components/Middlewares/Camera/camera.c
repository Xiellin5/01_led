#include "camera.h"
#include "uart.h"
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUF_SIZE 1024

static const uint8_t FRAME_HEADER[4] = {0xAA, 0x55, 0xAA, 0x55};

// 实例化图像结构体
static Camera_Image_t camera_image = {
    .length = 0,
    .is_ready = false
};

typedef enum {
    STATE_WAIT_HEADER,
    STATE_READ_LEN,
    STATE_READ_DATA
} state_t;

// ================= 接收任务 =================
static void recv_task(void *arg)
{
    uint8_t rx_buf[BUF_SIZE];
    state_t state = STATE_WAIT_HEADER;
    int header_match = 0;
    uint8_t len_buf[4];
    int len_idx = 0;
    
    uint32_t frame_len = 0;
    uint32_t recv_len = 0;

    while (1)
    {
        // 使用封装好的 UART 读取函数
        int n = UART_Read_Bytes(rx_buf, BUF_SIZE, 100);
        if (n <= 0) continue;

        for (int i = 0; i < n; i++)
        {
            uint8_t byte = rx_buf[i];

            switch (state)
            {
                // ================= 等待帧头 =================
                case STATE_WAIT_HEADER:
                    if (byte == FRAME_HEADER[header_match])
                    {
                        header_match++;
                        if (header_match == 4)
                        {
                            header_match = 0;
                            state = STATE_READ_LEN;
                            len_idx = 0;
                        }
                    }
                    else
                    {
                        header_match = (byte == FRAME_HEADER[0]) ? 1 : 0;
                    }
                    break;

                // ================= 读取长度 =================
                case STATE_READ_LEN:
                    len_buf[len_idx++] = byte;

                    if (len_idx == 4)
                    {
                        frame_len = len_buf[0] | (len_buf[1] << 8) | (len_buf[2] << 16) | (len_buf[3] << 24);

                        // ⭐ 合法性检查（关键）
                        if (frame_len < 500 || frame_len > MAX_FRAME)
                        {
                            printf("❌ 长度非法: %" PRIu32 "\n", frame_len);
                            state = STATE_WAIT_HEADER;
                        }
                        else
                        {
                            recv_len = 0;
                            state = STATE_READ_DATA;
                        }
                    }
                    break;

                // ================= 读取数据 =================
                case STATE_READ_DATA:
                    // 仅当上一帧被处理完后，才接收新帧写入，保护内存不被错乱复写
                    if (!camera_image.is_ready) 
                    {
                        camera_image.data[recv_len] = byte;
                    }
                    recv_len++;

                    // ⭐ 防止越界（工业级必须）
                    if (recv_len >= MAX_FRAME)
                    {
                        printf("❌ 越界丢弃\n");
                        state = STATE_WAIT_HEADER;
                        break;
                    }

                    if (recv_len >= frame_len)
                    {
                        // 只有处于未就绪状态才更新数据，防止多线程冲突
                        if (!camera_image.is_ready)
                        {
                            // ⭐ JPEG头检查（替代CRC）
                            if (camera_image.data[0] == 0xFF && camera_image.data[1] == 0xD8)
                            {
                                camera_image.length = frame_len;
                                camera_image.is_ready = true; // 触发标志位
                            }
                            else
                            {
                                printf("❌ 非JPEG数据\n");
                            }
                        }
                        state = STATE_WAIT_HEADER;
                    }
                    break;
            }
        }
    }
}

// ================= 摄像头功能初始化 =================
void Camera_Init(void)
{
    // 创建后台接收任务，优先级设为 10（偏高，防止丢包），绑定到核心 1
    xTaskCreatePinnedToCore(recv_task, "camera_recv", 8192, NULL, 10, NULL, 1);
}

// ================= 获取图像就绪状态 =================
bool Camera_Is_Ready(void)
{
    return camera_image.is_ready;
}

// ================= 获取完整图像结构体指针 =================
Camera_Image_t* Camera_Get_Image(void)
{
    return &camera_image;
}

// ================= 清除标志位，允许接收下一帧 =================
void Camera_Clear_Flag(void)
{
    camera_image.is_ready = false;
}