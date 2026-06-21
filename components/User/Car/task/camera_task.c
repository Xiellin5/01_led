#include "camera_task.h"
#include "camera_spi_slave.h"
#include "my_queue.h"          // 引入队列头文件
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <string.h>
#include "esp_cache.h"         // 引入 ESP-IDF 缓存管理头文件

// 声明静态全局变量暂存最新 AI 结果
static ai_result_t latest_ai_res; // 保存最新的AI检测结果
static bool has_ai_res = false;   // 标记：是否有新的AI结果要发给OpenMV

// 日志标签
static const char *TAG = "camera_task";
static const char *TAG_TEST = "test_consumer";

// 双DMA缓冲区（2块）：一块接收图片，一块给AI处理，交替使用不卡顿
static uint8_t *dma_buf[2] = {NULL, NULL};

/**
 * @brief 释放内存（防止复位时泄漏）
 */
void camera_task_memory_cleanup(void)
{
    for (int i = 0; i < 2; i++) {
        if (dma_buf[i] != NULL) {
            heap_caps_free(dma_buf[i]);   // 释放DMA内存
            dma_buf[i] = NULL;            // 指针置空
            ESP_LOGI(TAG, "Freed dma_buf[%d]", i);
        }
    }
}

/**
 * @brief 相机数据采集并发送任务
 */
void camera_task(void *pvParameters)
{
    esp_err_t err = camera_spi_slave_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "spi init failed");
        vTaskDelete(NULL);    // 初始化失败，删除自己这个任务
        return;
    }

    // 循环申请2 块 DMA 内存（双缓冲区）
    for (int i = 0; i < 2; i++) {
        dma_buf[i] = heap_caps_aligned_alloc(
            64,                                    // 64字节对齐（DMA强制要求）
            MAX_IMG_SIZE,                          // 大小20KB
            MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL   // DMA可用 + 内部SRAM
        );
        if (!dma_buf[i])                           // 申请失败
        {
            ESP_LOGE(TAG, "dma alloc failed");
            camera_task_memory_cleanup();          // 释放已申请内存
            vTaskDelete(NULL);                     // 删除任务
            return;
        }
    }

    ESP_LOGI(TAG, "camera task start");

    int buf_idx = 0;                              // 双缓冲索引：0 和 1 交替切换

    while (1)
    {
        // 1. 尝试非阻塞获取高层 【AI 结果队列】中的最新数据
        ai_result_t temp_res;
        if (ai_result_queue_receive(&temp_res, 0))  // 0=不等待
        {
            latest_ai_res = temp_res;               // 保存最新AI结果
            has_ai_res = true;                      // 标记有新结果要发
        }

        uint32_t img_len = 0;        // 实际收到的图片长度

        // 2. 调用通用的底层的 SPI 接收接口：
        // 同时把AI结果回传给OpenMV
        esp_err_t ret = camera_spi_slave_receive_image(
            dma_buf[buf_idx], 
            &img_len, 
            has_ai_res ? &latest_ai_res : NULL,
            has_ai_res ? sizeof(ai_result_t) : 0
        );

        if (ret == ESP_OK && img_len > 4)
        {
            has_ai_res = false; // 已经发给OpenMV，清除标记

            // 【核心修复】：必须先刷新 Cache（将 DMA 接收的数据从物理内存同步到 CPU）
            // 确保 CPU 在接下来的读取中能够获取到物理内存中真实的 JPEG 头尾字节，防止因 Cache 过期误判为 00 00
            uint32_t aligned_size = (img_len + 63) & (~63);
            esp_cache_msync(dma_buf[buf_idx], aligned_size, ESP_CACHE_MSYNC_FLAG_DIR_M2C);

			// ==================== JPEG 完整性校验 ====================
            // 取JPEG头：FF D8
            uint8_t head0 = dma_buf[buf_idx][0];
            uint8_t head1 = dma_buf[buf_idx][1];
			// 取JPEG尾：FF D9
            uint8_t tail0 = dma_buf[buf_idx][img_len - 2];
            uint8_t tail1 = dma_buf[buf_idx][img_len - 1];

			 // 如果头和尾都正确 → 图片完整
            if (head0 == 0xFF && head1 == 0xD8 && tail0 == 0xFF && tail1 == 0xD9)
            {
				// 图片起始地址 + 结束地址
                const uint8_t *start = dma_buf[buf_idx];
                const uint8_t *end = dma_buf[buf_idx] + img_len;

				// ==================== 把图片送入队列给AI处理 ====================
                if (jpeg_queue_send(start, end, pdMS_TO_TICKS(100)))   //最多等 100ms,发不出去就丢帧，缓冲区立刻释放
                {
                    ESP_LOGI(TAG, "Send to Queue SUCCESS! Size: %lu bytes", img_len);
                    buf_idx = 1 - buf_idx;    // 切换缓冲（双缓冲核心）
                }
                else
                {
                    ESP_LOGW(TAG, "Send to Queue TIMEOUT!");
                }
            }
            else
            {
				// JPEG损坏（SPI干扰/传输错误）
                ESP_LOGE(TAG, "JPEG Corrupted by Noise! Frame Dropped. Head: %02X %02X, Tail: %02X %02X", 
                         head0, head1, tail0, tail1);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));      // 小延时，让系统切换任务
    }
}

/**
 * @brief 临时测试用的消费者任务（替代 ai_task）
 *        作用：从队列取出图片，打印大小、头尾，测试队列是否正常
 */
void test_consumer_task(void *pvParameters)
{
    const uint8_t *start = NULL;
    const uint8_t *end = NULL;

    ESP_LOGI(TAG_TEST, "Test consumer task started...");

    while (1)
    {
		// 阻塞等待队列有图片
        if (jpeg_queue_receive(&start, &end, portMAX_DELAY))
        {
            uint32_t recv_len = end - start;   // 计算图片长度
            
            ESP_LOGW(
                TAG_TEST, 
                "Successfully read from Queue! Size=%lu, Head=[%02X %02X], Tail=[%02X %02X]",
                recv_len,
                start[0], start[1],
                start[recv_len - 2], start[recv_len - 1]
            );
        }
    }
}