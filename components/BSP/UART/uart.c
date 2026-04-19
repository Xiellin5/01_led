#include "uart.h"
#include "driver/uart.h"

// ================= UART初始化 =================
void UART_Init(void)
{
    uart_config_t cfg = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART_PORT, &cfg);
    uart_set_pin(UART_PORT, ESP32_TX_PIN, ESP32_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // 分配 64KB 的环形接收缓冲区，确保 3M 波特率下不丢包
    uart_driver_install(UART_PORT, 65536, 0, 0, NULL, 0);
}

// ================= UART读取封装 =================
int UART_Read_Bytes(uint8_t *buf, uint32_t length, uint32_t timeout_ms)
{
    return uart_read_bytes(UART_PORT, buf, length, pdMS_TO_TICKS(timeout_ms));
}