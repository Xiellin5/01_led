#ifndef CAMERA_SPI_SLAVE_H
#define CAMERA_SPI_SLAVE_H

#include <stdint.h>
#include "esp_err.h"

// 将最大图片尺寸定义放在头文件，方便主程序使用
#define MAX_IMG_SIZE    (64 * 1024) 

/**
 * @brief 初始化 SPI 从机及握手引脚，并分配头部的 DMA 内存
 */
esp_err_t camera_spi_slave_init(void);

/**
 * @brief 接收一次完整的图像流 (包含 Header 解析和图像主体接收)
 * @param img_buf [in] 用于接收图片的缓冲区 (必须是 64 字节对齐且具有 DMA 能力)
 * @param out_len [out] 成功接收到的实际图片长度
 * @return esp_err_t 如果成功返回 ESP_OK
 */
esp_err_t camera_spi_slave_receive_image(uint8_t *img_buf, uint32_t *out_len);

#endif // CAMERA_SPI_SLAVE_H