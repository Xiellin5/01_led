#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "camera_spi_slave.h"

static const char *TAG = "SPI_IMAGE_RECV";

void app_main(void) {
    // 1. 调用子文件的初始化函数
    ESP_ERROR_CHECK(camera_spi_slave_init());
    ESP_LOGI(TAG, "SPI Slave Image Receiver Ready...");

    // 2. 在主程序中申请图片缓冲区 (因为处理逻辑主要在 main 中发生)
    uint8_t *img_buf = heap_caps_aligned_alloc(64, MAX_IMG_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (img_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate image memory!");
        return;
    }

    while (1) {
        uint32_t img_len = 0;
        
        // 3. 调用底层驱动接收图片
        esp_err_t ret = camera_spi_slave_receive_image(img_buf, &img_len);

        // 与原版逻辑完全一致：只有当返回 ESP_OK 时才打印
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "JPEG Received Successfully! Size: %ld bytes", img_len);
            
            // 这里可以添加对图片的处理逻辑（如保存到SD卡、显示在屏上等）
            // 图片数据在 img_buf 中，有效长度为 img_len
        }

        // 维持原版的延时节奏
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}