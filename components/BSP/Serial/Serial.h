/**
 * @file serial.h
 * @brief Low level functions for sending and receiving bytes via UART (ESP32-P4)
 * @note Compatible with GRBL serial interface, ported to ESP32-P4 UART driver
 * @copyright GNU General Public License v3.0
 */
#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/uart.h"

/**
 * @def USART_UX
 * @brief ESP32-P4 UART port number (UART_NUM_0 for default)
 */
#define USART_UX UART_NUM_0

/**
 * @def USART_TX_GPIO_PIN
 * @brief TX pin for UART (ESP32-P4 GPIO37 as example)
 */
#define USART_TX_GPIO_PIN GPIO_NUM_37

/**
 * @def USART_RX_GPIO_PIN
 * @brief RX pin for UART (ESP32-P4 GPIO38 as example)
 */
#define USART_RX_GPIO_PIN GPIO_NUM_38

/**
 * @def USART_BOUD_RATE
 * @brief UART baud rate (115200 for GRBL default)
 */
#define USART_BOUD_RATE 115200

/**
 * @def RX_BUFFER_SIZE
 * @brief RX ring buffer size (GRBL default 128)
 */
#ifndef RX_BUFFER_SIZE
#define RX_BUFFER_SIZE 128
#endif

/**
 * @def TX_BUFFER_SIZE
 * @brief TX ring buffer size (GRBL adaptive size)
 */
#ifndef TX_BUFFER_SIZE
#ifdef USE_LINE_NUMBERS
#define TX_BUFFER_SIZE 112
#else
#define TX_BUFFER_SIZE 104
#endif
#endif

/**
 * @def SERIAL_NO_DATA
 * @brief Return value when no data in RX buffer
 */
#define SERIAL_NO_DATA 0xff

/**
 * @brief Initialize UART port for GRBL serial communication
 * @details Configure UART baud rate, data bits, stop bits, parity, and install driver
 * @param None
 * @return None
 */
void serial_init();

/**
 * @brief Write one byte to TX serial buffer (async via ESP32 UART driver)
 * @param data Byte to send
 * @return None
 */
void serial_write(uint8_t data);

/**
 * @brief Read one byte from RX serial buffer
 * @return uint8_t: Byte from buffer, or SERIAL_NO_DATA if empty
 */
uint8_t serial_read();

/**
 * @brief Reset and empty RX serial buffer (for e-stop/reset)
 * @param None
 * @return None
 */
void serial_reset_read_buffer();

/**
 * @brief Get available bytes in RX buffer (free space)
 * @return uint8_t: Number of available bytes
 */
uint16_t serial_get_rx_buffer_available();

/**
 * @brief Get used bytes in RX buffer (deprecated, for classic status)
 * @return uint8_t: Number of used bytes
 */
uint16_t serial_get_rx_buffer_count();

/**
 * @brief Get used bytes in TX buffer (for debugging TX bottleneck)
 * @return uint8_t: Number of used bytes
 */
uint16_t serial_get_tx_buffer_count();

#endif // SERIAL_H