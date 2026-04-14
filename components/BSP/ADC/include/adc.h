/**
 ****************************************************************************************************
 * @file        adc1.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       ADC驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32-P4 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

/* 管脚声明与配置 */
#define ADC_UNIT_X ADC_UNIT_1        /* ADC单元:ADC1(ADC_UNIT_1)/ADC2(ADC_UNIT_2) */
#define ADC_CHAN ADC_CHANNEL_3       /* ADC1通道3对应GPIO19 */
#define ADC_ATTEN ADC_ATTEN_DB_12    /* ADC衰减 */
#define ADC_BITWIDTH ADC_BITWIDTH_12 /* ADC分辨率 */

/* 函数声明 */
void adc_init(void);                        /* 初始化ADC */
void adc_deinit(void);                      /* 关闭ADC */
int adc_get_result_voltage(uint32_t times); /* 获取ADC滤波后的电压数值 */
