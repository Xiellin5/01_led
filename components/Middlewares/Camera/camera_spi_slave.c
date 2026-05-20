#include "camera_spi_slave.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <string.h>

// 宏定义放在 .c 文件中，对外隐藏
#define GPIO_CS         20
#define GPIO_SCLK       21
#define GPIO_MISO       22
#define GPIO_MOSI       23
#define GPIO_HANDSHAKE  24

#define SPI_HOST        SPI2_HOST

// 将 header_buf 声明为静态全局变量，与原版 main.c 中申请一次的行为保持一致
static uint8_t *header_buf = NULL;

esp_err_t camera_spi_slave_init(void) {
    // 1. 初始化握手信号线 (GPIO 24)
    gpio_config_t io_conf = { 
        .intr_type = GPIO_INTR_DISABLE, 
        .mode = GPIO_MODE_OUTPUT, 
        .pin_bit_mask = (1ULL << GPIO_HANDSHAKE) 
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_HANDSHAKE, 0);

    // 2. 申请 Header 的 64 字节对齐 DMA 内存
    header_buf = heap_caps_aligned_alloc(64, 64, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (header_buf == NULL) {
        return ESP_ERR_NO_MEM;
    }
    memset(header_buf, 0, 64);

    // 3. 配置 SPI 总线
    spi_bus_config_t buscfg = { 
        .mosi_io_num = GPIO_MOSI, 
        .miso_io_num = GPIO_MISO, 
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1, 
        .quadhd_io_num = -1, 
        .max_transfer_sz = MAX_IMG_SIZE 
    };

    spi_slave_interface_config_t slvcfg = { 
        .mode = 0, 
        .spics_io_num = GPIO_CS, 
        .queue_size = 3 
    };

    return spi_slave_initialize(SPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
}

esp_err_t camera_spi_slave_receive_image(uint8_t *img_buf, uint32_t *out_len) {
    // 每次接收前清空头部缓冲，防止上一次的残留数据干扰判断
    memset(header_buf, 0, 64);

    // --- 步骤 1: 接收 Header (64字节) ---
    spi_slave_transaction_t t_head = { .length = 64 * 8, .rx_buffer = header_buf };
    
    gpio_set_level(GPIO_HANDSHAKE, 1);
    esp_err_t ret = spi_slave_transmit(SPI_HOST, &t_head, portMAX_DELAY);
    gpio_set_level(GPIO_HANDSHAKE, 0);

    if (ret != ESP_OK) {
        return ret;
    }

    if (header_buf[0] == 0xFE) {
        // 解析图片长度
        uint32_t img_len = (header_buf[1] << 16) | (header_buf[2] << 8) | header_buf[3];
        
        if (img_len > 0 && img_len <= MAX_IMG_SIZE) {
            // 计算对齐到 64 字节的传输长度
            uint32_t aligned_len = (img_len + 63) & (~63); 
            
            // --- 步骤 2: 接收图片主体 ---
            spi_slave_transaction_t t_img = { 
                .length = aligned_len * 8, 
                .rx_buffer = img_buf 
            };
            
            gpio_set_level(GPIO_HANDSHAKE, 1);
            ret = spi_slave_transmit(SPI_HOST, &t_img, portMAX_DELAY);
            gpio_set_level(GPIO_HANDSHAKE, 0);

            if (ret == ESP_OK) {
                *out_len = img_len; // 赋值传出实际长度
                return ESP_OK;
            }
            return ret;
        } else {
            return ESP_ERR_INVALID_SIZE; // 长度越界
        }
    }
    
    return ESP_ERR_INVALID_STATE; // 头标志不对 (不是 0xFE)
}