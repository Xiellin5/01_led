#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_cache.h"         // 【新增】引入缓存同步头文件
#include "camera_spi_slave.h"
#include "sd_card.h"           // 引入新增的 SD 卡头文件

static const char *TAG = "SPI_IMAGE_RECV";

void app_main(void) {
    // 1. 调用子文件的初始化函数 (SPI 从机)
    ESP_ERROR_CHECK(camera_spi_slave_init());
    ESP_LOGI(TAG, "SPI Slave Image Receiver Ready...");

    // 2. 初始化 SD 卡
    bool sd_ready = false;
    if (sd_card_init() == ESP_OK) {
        sd_ready = true;
    } else {
        ESP_LOGE(TAG, "SD Card Init Failed! Images will not be saved.");
        // 这里不直接 return，允许即便无SD卡也能维持通信，方便调试
    }

    // 3. 在主程序中申请图片缓冲区 (放置在 DMA & INTERNAL 内存区域)
    uint8_t *img_buf = heap_caps_aligned_alloc(64, MAX_IMG_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (img_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate image memory!");
        return;
    }

    while (1) {
        uint32_t img_len = 0;
        
        // 4. 【修改参数】：调用底层驱动接收图片（因无 AI 结果，tx_data 传入 NULL，tx_len 传入 0）
        esp_err_t ret = camera_spi_slave_receive_image(img_buf, &img_len, NULL, 0);

        // 只有当返回 ESP_OK 且有实际数据时才处理
        if (ret == ESP_OK && img_len > 0) {
            
            // 【核心安全保护】：在 CPU 读取并保存文件之前，先将 DMA 写入的数据从物理内存同步到 CPU Cache
            // 这能绝对保证保存到 SD 卡里的图片不损坏、能够正常在电脑上双击打开
            uint32_t aligned_size = (img_len + 63) & (~63);
            esp_cache_msync(img_buf, aligned_size, ESP_CACHE_MSYNC_FLAG_DIR_M2C);

            ESP_LOGI(TAG, "JPEG Received Successfully! Size: %lu bytes", img_len);
            
            // 5. 如果 SD 卡挂载成功，则保存图片
            if (sd_ready) {
                sd_card_save_image(img_buf, img_len);
            }
        }

        // 维持一定的延时节奏，让出 CPU
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}