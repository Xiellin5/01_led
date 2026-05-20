#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdint.h>
#include "esp_err.h"

/**
 * @brief 初始化 SD 卡并挂载 FAT 文件系统
 */
esp_err_t sd_card_init(void);

/**
 * @brief 将图片数据保存到 SD 卡中 (自动按序号命名)
 * @param img_data 图片数据指针
 * @param img_len 图片数据长度
 */
esp_err_t sd_card_save_image(uint8_t *img_data, uint32_t img_len);

#endif