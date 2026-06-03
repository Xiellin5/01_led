#include "camera_spi_slave.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <string.h>
#include "esp_cache.h"

// 宏定义放在 .c 文件中，对外隐藏
#define GPIO_CS         20
#define GPIO_SCLK       21
#define GPIO_MISO       22
#define GPIO_MOSI       23
#define GPIO_HANDSHAKE  24

#define SPI_HOST        SPI2_HOST

// 声明接收和发送的静态 DMA 缓冲区指针
// 静态变量：只能在本文件访问，用于存放64字节协议头
// header_buf：接收OpenMV发来的头信息（0xFE + 图片长度）
static uint8_t *header_buf = NULL;
// tx_header_buf：发送给OpenMV的AI结果头（0xFD/0x00 + 数据）
// 必须是DMA内存，64字节对齐
static uint8_t *tx_header_buf = NULL; 



esp_err_t camera_spi_slave_init(void) {
    // 1. 初始化握手信号线 (GPIO 24)
    gpio_config_t io_conf = { 
        .intr_type = GPIO_INTR_DISABLE,            // 关闭中断
        .mode = GPIO_MODE_OUTPUT,                  // 输出模式
        .pin_bit_mask = (1ULL << GPIO_HANDSHAKE)   // 选中引脚24 
    };
    gpio_config(&io_conf);                         // 应用GPIO配置
    gpio_set_level(GPIO_HANDSHAKE, 0);             // 默认拉低：未准备好

    // 2. 【核心修复】：同时申请接收与发送 Header 的 64 字节对齐 DMA 内存
    header_buf = heap_caps_aligned_alloc(
		64, // 对齐要求：必须64字节对齐
		64, // 申请大小：64字节
		MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);  // 内存属性：DMA可用 + 内部RAM
	// 同样申请一块 DMA 内存，用于发送 AI 结果给 OpenMV	
    tx_header_buf = heap_caps_aligned_alloc(64, 64, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    
    // 安全性检查：如果任意一个申请失败，安全释放已申请的内存防止泄漏
    if (header_buf == NULL || tx_header_buf == NULL) {
        if (header_buf != NULL) {
            heap_caps_free(header_buf);
            header_buf = NULL;
        }
        if (tx_header_buf != NULL) {
            heap_caps_free(tx_header_buf);
            tx_header_buf = NULL;
        }
        return ESP_ERR_NO_MEM;// 返回：内存不足错误
    }
    
    // 初始化清空缓冲区
    memset(header_buf, 0, 64);
    memset(tx_header_buf, 0, 64);

    // 3. 配置 SPI 从机总线
    spi_bus_config_t buscfg = { 
        .mosi_io_num = GPIO_MOSI, 
        .miso_io_num = GPIO_MISO, 
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1, 
        .quadhd_io_num = -1, 
        .max_transfer_sz = MAX_IMG_SIZE // 最大传输尺寸 = 20KB
    };
    // SPI从机模式配置
    spi_slave_interface_config_t slvcfg = { 
        .mode = 0,                  // SPI模式0（CPOL=0, CPHA=0）
        .spics_io_num = GPIO_CS,    // 片选引脚
        .queue_size = 3             // 事务队列长度
    };
    // 初始化SPI从机，并自动分配DMA通道
    return spi_slave_initialize(SPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
}

// 接收函数实现：
esp_err_t camera_spi_slave_receive_image(uint8_t *img_buf, uint32_t *out_len, const void *tx_data, size_t tx_len) {
	// 每次传输前清空接收和发送头缓冲区
    memset(header_buf, 0, 64);    //memset(指针, 要设置的值, 长度);
    memset(tx_header_buf, 0, 64); // 每次清理发送缓冲（此时编译器已知该指针安全且不为 NULL）

    // 【通用数据载入】如果高层传入了有效的数据，拼装进 64 字节头中
    if (tx_data != NULL && tx_len > 0) {
        tx_header_buf[0] = 0xFD; // 魔法标识符：协议头：0xFD = 有有效检测结果
        
        // 限制最大拷贝长度为 60 字节（保留4字节给协议头），防止越界破坏 64 字节的头协议格式
        size_t copy_len = (tx_len > 60) ? 60 : tx_len;
        memcpy(&tx_header_buf[4], tx_data, copy_len); // 把AI数据拷贝到发送缓冲区，从第4字节开始放
    } else {
        tx_header_buf[0] = 0x00; // 协议头：0x00 表示当前无无检测结果，无有效传输数据
    }

    // 刷新 Cache，确保 DMA 发送最新的内存数据
    esp_cache_msync(tx_header_buf, 64, ESP_CACHE_MSYNC_FLAG_DIR_C2M);

    // --- 步骤 1: 接收 Header (双工发送 tx_header_buf) ---
    spi_slave_transaction_t t_head = { 
        .length = 64 * 8,          // 传输位数：64字节 × 8 = 512位
        .rx_buffer = header_buf,   // 接收头存入这里
        .tx_buffer = tx_header_buf // 双工机制（同时发送AI结果）
    };
    
    gpio_set_level(GPIO_HANDSHAKE, 1);// 拉高握手信号 → 告诉OpenMV：ESP32已准备好，可以发送数据
    esp_err_t ret = spi_slave_transmit(SPI_HOST, &t_head, portMAX_DELAY);// 阻塞式传输：直到64字节收发完成才往下走
    gpio_set_level(GPIO_HANDSHAKE, 0);// 拉低握手信号 → 传输结束
    
	// 如果SPI传输失败，直接返回错误
    if (ret != ESP_OK) {
        return ret;
    }
	
	// 【ESP32 必须】刷新缓存，让DMA收到的数据同步给CPU
    esp_cache_msync(header_buf, 64, ESP_CACHE_MSYNC_FLAG_DIR_M2C);

	// ------------------- 解析协议头 -------------------
    // 判断第一个字节是不是 0xFE（OpenMV发图的固定包头）
    if (header_buf[0] == 0xFE) { // 还原3字节 → 32位图片长度
        uint32_t img_len = (header_buf[1] << 16) | (header_buf[2] << 8) | header_buf[3];
        
		// 检查长度是否合法（大于0，不超过20KB）
        if (img_len > 0 && img_len <= MAX_IMG_SIZE) {
			// 计算64字节对齐后的长度（SPI DMA要求整64字节传输）
            uint32_t aligned_len = (img_len + 63) & (~63); 
            
            // --- 步骤 2: 接收图片主体 ---
            spi_slave_transaction_t t_img = { 
                .length = aligned_len * 8,      // 对齐后的长度（位）
                .rx_buffer = img_buf            // 图片存入外部传入的缓冲区
            };
            
			// 再次拉高握手，通知OpenMV发送图片
            gpio_set_level(GPIO_HANDSHAKE, 1);
            ret = spi_slave_transmit(SPI_HOST, &t_img, portMAX_DELAY);
            gpio_set_level(GPIO_HANDSHAKE, 0);

			// 如果接收成功
            if (ret == ESP_OK) {
                *out_len = img_len;      // 输出真实的图片长度
                return ESP_OK;           // 返回成功
            }
            return ret;                  // 返回SPI错误
        } else {
            return ESP_ERR_INVALID_SIZE; // 图片长度不合法
        }
    }
    return ESP_ERR_INVALID_STATE;        // 包头不是0xFE，协议错误
}