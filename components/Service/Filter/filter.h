#ifndef __FILTER_H
#define __FILTER_H
#include <stdint.h>
#include "math.h"
#include "string.h"
// 限幅
void amplitude_limit_filter(const uint16_t *raw_data, uint16_t *filtered_data, uint16_t data_len, uint16_t limit);

// 中位数
uint16_t median_filter(const uint16_t *raw_data, uint16_t data_len, uint16_t start_idx, uint16_t sample_num);

/* ====================== 算术平均滤波法 ====================== */
/**
 * @brief  通用版算术平均滤波函数
 * @retval 滤波后的算术平均值；参数非法返回0
 */
uint16_t arithmetic_average_filter(const uint16_t *raw_data, uint16_t data_len,
                                   uint16_t start_idx, uint16_t sample_num);

/* ====================== 递推平均（滑动平均）滤波法 ====================== */
/**
 * @brief  通用版递推平均滤波函数（单次滑动计算）
 * @retval 滤波后的滑动平均值；参数非法返回0
 */
uint16_t recursive_average_filter(uint16_t *queue_buf, uint16_t queue_len, uint16_t new_value);

/* ====================== 中位值平均滤波法（防脉冲干扰平均滤波） ====================== */
/**
 * @brief  通用版中位值平均滤波函数
 * @retval 滤波后的平均值；参数非法返回0
 */
uint16_t median_average_filter(const uint16_t *raw_data, uint16_t data_len,
                               uint16_t start_idx, uint16_t sample_num);

/* ====================== 限幅平均滤波法 ====================== */
/**
 * @brief  通用版限幅平均滤波函数（单次滑动计算）
 * @retval 滤波后的平均值；参数非法返回0
 */
uint16_t amplitude_limit_average_filter(uint16_t *queue_buf, uint16_t queue_len,
                                        uint16_t new_value, uint16_t limit);

/* ====================== 一阶滞后滤波法 ====================== */
/**
 * @brief  通用版一阶滞后滤波函数
 * @retval 本次滤波结果；参数非法返回last_result
 */
uint16_t first_order_lag_filter(uint16_t new_value, uint16_t last_result, float alpha);

/* ====================== 加权递推平均滤波法 ====================== */
/**
 * @brief  通用版加权递推平均滤波函数
 * @retval 加权平均结果；参数非法返回0
 */
uint16_t weighted_recursive_average_filter(uint16_t *queue_buf, uint16_t queue_len,
                                           const uint16_t *coe, uint32_t sum_coe,
                                           uint16_t new_value);

/* ====================== 消抖滤波法 ====================== */
/**
 * @brief  通用版消抖滤波函数
 * @param  counter: 滤波计数器（需全局/静态，保持状态）
 * @retval 当前有效值；参数非法返回last_valid
 */
uint16_t debounce_filter(uint16_t new_value, uint16_t *last_valid,
                         uint16_t *counter, uint16_t max_count);

/* ====================== 限幅消抖滤波法 ====================== */
/**
 * @brief  通用版限幅消抖滤波函数
 * @param  counter: 滤波计数器（需全局/静态，保持状态）
 * @retval 当前有效值；参数非法返回last_valid
 */
uint16_t amplitude_limit_debounce_filter(uint16_t new_value, uint16_t *last_valid,
                                         uint16_t *counter, uint16_t limit,
                                         uint16_t max_count);
#endif /* __FILTER_H */
