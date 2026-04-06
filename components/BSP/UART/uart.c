#include "uart.h"
void uart_init(uint32_t baudrate)
{
    uart_config_t uart_config = {
        .baud_rate = baudrate,
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
    ESP_ERROR_CHECK(uart_driver_install(USART_UX, RX_BUF_SIZE, TX_BUF_SIZE, 0, NULL, 0));
};