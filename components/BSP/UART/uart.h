#pragma once
#include "driver/uart.h"
#include "esp_system.h"
#include "driver/gpio.h"
/* 引脚和串口定义 */
#define USART_UX UART_NUM_0
#define USART_TX_GPIO_PIN GPIO_NUM_37
#define USART_RX_GPIO_PIN GPIO_NUM_38
/* 串口接收相关定义 */
#define RX_BUF_SIZE 1024
#define TX_BUF_SIZE RX_BUF_SIZE

void uart_init(uint32_t baudrate);