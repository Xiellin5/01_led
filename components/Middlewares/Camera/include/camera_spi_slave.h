//#ifndef CAMERA_SPI_SLAVE_H
//#define CAMERA_SPI_SLAVE_H

#pragma once
#include "esp_err.h"
#include <stdint.h>   // 引入 size_t 支持
#include "esp_err.h"

// 将最大图片尺寸定义放在头文件，方便主程序使用
#define MAX_IMG_SIZE    (20 * 1024) 

/**
 * @brief 初始化 SPI 从机及握手引脚，并分配头部的 DMA 内存
 */
esp_err_t camera_spi_slave_init(void);

/**
 * @brief 接收一次完整的图像流 (包含 Header 解析和图像主体接收)
 * @param img_buf [in]  用于接收图片的缓冲区 (必须是 64 字节对齐且具有 DMA 能力)
 * @param out_len [out] 成功接收到的实际图片长度
 * @param tx_data [in]  要通过 SPI 双工模式同步发送给 OpenMV 的数据（AI 检测结果）
 * 						支持任意类型（使用 void* 通用指针实现解耦）
 * @param tx_len  [in]  要发送给 OpenMV 的数据长度（固定为 64 字节，与协议对齐）
 * @return esp_err_t 如果成功返回 ESP_OK
 */
//esp_err_t camera_spi_slave_receive_image(uint8_t *img_buf, uint32_t *out_len);
// 【解耦设计】使用万能指针 void*，底层无需感知高层的数据结构
esp_err_t camera_spi_slave_receive_image(uint8_t *img_buf, uint32_t *out_len, const void *tx_data, size_t tx_len);

//#endif // CAMERA_SPI_SLAVE_H