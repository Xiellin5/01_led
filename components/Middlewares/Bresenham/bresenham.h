#ifndef BRESENHAM_H
#define BRESENHAM_H

#include <stdint.h>

/**
 * @brief 回调函数类型，用于处理每个生成的像素点
 * @param x 像素x坐标
 * @param y 像素y坐标
 */
typedef void (*bresenham_callback_t)(int x, int y);

/**
 * @brief 使用Bresenham整数算法生成直线上的所有点
 * @param x0 起点x
 * @param y0 起点y
 * @param x1 终点x
 * @param y1 终点y
 * @param callback 每生成一个点调用的回调函数
 * @note 算法支持任意象限，基于整数运算
 */
void bresenham_line(int x0, int y0, int x1, int y1, bresenham_callback_t callback);

#endif // BRESENHAM_H