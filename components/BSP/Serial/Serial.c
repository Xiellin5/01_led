/*
  serial.c - Low level functions for sending and recieving bytes via the serial port
  Part of Grbl

  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "serial.h"
#define RX_RING_BUFFER (RX_BUFFER_SIZE * 2)
#define TX_RING_BUFFER (TX_BUFFER_SIZE * 2)

static volatile uint8_t serial_rx_date = 0;

uint16_t serial_get_rx_buffer_available()
{
    uint16_t length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(USART_UX, (size_t *)&length));
    return (RX_RING_BUFFER - length);
}

uint16_t serial_get_rx_buffer_count()
{
    uint16_t length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(USART_UX, (size_t *)&length));
    return length;
}

uint16_t serial_get_tx_buffer_count()
{
    uint16_t length = 0;
    ESP_ERROR_CHECK(uart_get_tx_buffer_free_size(USART_UX, (size_t *)&length));
    return length;
}

void serial_init()
{
    uart_config_t uart_config = {
        .baud_rate = USART_BOUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .parity = UART_PARITY_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
        .stop_bits = UART_STOP_BITS_1,
    };

    ESP_ERROR_CHECK(uart_param_config(USART_UX, &uart_config)); // init
    // Set UART pins
    ESP_ERROR_CHECK(uart_set_pin(USART_UX, USART_TX_GPIO_PIN, USART_RX_GPIO_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // Install UART driver for interrupt-driven reads and writes
    ESP_ERROR_CHECK(uart_driver_install(USART_UX, RX_RING_BUFFER, TX_RING_BUFFER, 0, NULL, 0));
}

void serial_write(uint8_t data)
{
    uart_write_bytes(USART_UX, &data, 1); // 发送一个字节（没发送成功一定不会返回）
}

uint8_t serial_read()
{
    uint16_t length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(USART_UX, (size_t *)&length));
    if (length <= 0)
    {
        return SERIAL_NO_DATA;
    }
    uart_read_bytes(USART_UX, (void *)&serial_rx_date, 1, pdMS_TO_TICKS(10));
    return (serial_rx_date <= 0) ? SERIAL_NO_DATA : serial_rx_date;
}

void serial_reset_read_buffer()
{
    uart_flush(USART_UX);
}
