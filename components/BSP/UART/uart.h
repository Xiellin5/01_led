#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

// UART 引脚与参数定义
#define UART_PORT      UART_NUM_1
#define ESP32_TX_PIN   17
#define ESP32_RX_PIN   18
#define BAUD_RATE      3000000

// 函数声明
void UART_Init(void);
int UART_Read_Bytes(uint8_t *buf, uint32_t length, uint32_t timeout_ms);

#endif /* _UART_H_ */