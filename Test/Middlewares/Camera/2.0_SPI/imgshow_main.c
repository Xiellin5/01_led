#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "driver/uart.h" 
#include "driver/gpio.h"
#include "camera_spi_slave.h"

static const char *TAG = "SPI_IMAGE_RECV";

// 1. 修正引脚：TXD 对应 GPIO16，RXD 对应 GPIO15
#define TXD_PIN (GPIO_NUM_16)  // ESP32 TX2 -> 连接 USB-TTL RX
#define RXD_PIN (GPIO_NUM_15)  // ESP32 RX2 -> 连接 USB-TTL TX
#define UART_PORT_NUM (UART_NUM_2)
#define UART_BAUD_RATE (921600) // 恢复高速波特率以支持视频流
#define UART_BUF_SIZE (1024)

/**
 * @brief 初始化 UART 串口
 */
static void uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // 分配较大的发送/接收缓冲区
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUF_SIZE * 6, UART_BUF_SIZE * 6, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

void app_main(void) {
    // 初始化串口
    uart_init();
    ESP_LOGI(TAG, "UART initialized on TX: GPIO%d, RX: GPIO%d, Baudrate: %d", TXD_PIN, RXD_PIN, UART_BAUD_RATE);

    // 初始化 SPI
    ESP_ERROR_CHECK(camera_spi_slave_init());
    ESP_LOGI(TAG, "SPI Slave Image Receiver Ready...");

    // 申请图片缓冲区
    uint8_t *img_buf = heap_caps_aligned_alloc(64, MAX_IMG_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (img_buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate image memory!");
        return;
    }

    // 传输协议帧头
    const uint8_t frame_header[4] = {0xAA, 0x55, 0xAA, 0x55};

    while (1) {
        uint32_t img_len = 0;
        
        // 接收来自 OpenMV 的图片
        esp_err_t ret = camera_spi_slave_receive_image(img_buf, &img_len);

        // 成功接收到图片后，完整发送给 Python 端口
        if (ret == ESP_OK && img_len > 0) {
            ESP_LOGI(TAG, "JPEG Received Successfully! Size: %lu bytes. Sending...", img_len);
            
            // 发送 4 字节帧头
            uart_write_bytes(UART_PORT_NUM, (const char *)frame_header, 4);
            
            // 发送 4 字节的长度信息
            uart_write_bytes(UART_PORT_NUM, (const char *)&img_len, 4);
            
            // 发送图片主体数据
            uart_write_bytes(UART_PORT_NUM, (const char *)img_buf, img_len);
        }

        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}