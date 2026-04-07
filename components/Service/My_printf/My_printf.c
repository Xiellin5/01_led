#include <stdarg.h> // 处理可变参数的核心头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <My_printf.h>

static printer_t printer = {
	.init = NULL,
	.print_string = NULL,
	.is_registered = false,
};
// 定义缓冲区：存储拼接好的完整字符串
static uint8_t print_buf[512] = {0};
// 记录缓冲区当前写入位置（偏移量）
static int buf_offset = 0;

/**
 * @brief 注册打印函数
 *
 * @param init 初始化函数
 * @param print_string 字符串输出函数
 */
void print_register(void (*init)(void), void (*print_string)(uint8_t *str))
{
	printer.init = init;
	printer.print_string = print_string;
	printer.is_registered = true;
}

static uint8_t print_init_flag = 0; // 是否初始化printf标志位
static bool print_init()
{
	if (printer.is_registered == false)
	{
		return false; // 未注册打印函数，无法初始化
	}
	if (print_init_flag == 1)
	{
		return true; // 已初始化，不再重复初始化
	}
	printer.init();		 // 调用初始化函数
	print_init_flag = 1; // 设置初始化标志位
	return true;
}

/**
 * 输出任意数量的float参数，格式自动适配(采用FireWater格式)[输入时一定保证输入为float类型]：
 * - 1个参数 → %f\r\n
 * - 2个参数 → %f,%f\r\n
 * - n个参数 → %f,%f,...,%f\r\n
 * @param count 参数的个数（必须准确传入）
 * @param ... 待输出的float参数列表
 */
void print_FireWater(int count, ...)
{
	if (!print_init()) // 初始化printf
	{
		return;
	}
	memset(print_buf, 0, sizeof(print_buf)); // 清空缓冲区
	buf_offset = 0;							 // 重置缓冲区偏移量

	// 2. 初始化可变参数列表
	va_list args;
	va_start(args, count);

	// 3. 参数个数为0直接返回
	if (count <= 0)
	{
		va_end(args);
		return;
	}

	// 4. 拼接第一个浮点数（无逗号）
	float val = va_arg(args, double);
	// 格式化写入缓冲区，返回写入的字符长度
	buf_offset += sprintf((char *)print_buf + buf_offset, "%f", val);

	// 5. 拼接剩余浮点数（前面带逗号）
	for (int i = 1; i < count; i++)
	{
		val = va_arg(args, double);
		buf_offset += sprintf((char *)print_buf + buf_offset, ",%f", val);
	}

	// 6. 拼接结尾换行符
	sprintf((char *)print_buf + buf_offset, "\r\n");

	// 7. 【核心】调用注册的print_string输出完整字符串
	printer.print_string(print_buf);

	// 8. 释放可变参数列表
	va_end(args);
}
