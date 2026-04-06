#include <stdio.h>
#include "serial.h"
#include "unity.h"
void test_flush_before_read()
{
    int len = 0;
    uart_get_buffered_data_len(USART_UX, (size_t *)&len);
    printf("available data before read: %d\n", len);
    uart_flush(USART_UX);
    uart_get_buffered_data_len(USART_UX, (size_t *)&len);
    printf("available data after flush: %d\n", len);

    // 效果
    // 输入数据
    // available data before read: 2
    // available data after flush: 0
}

void test_read()
{
    uint8_t data = 0;
    // read
    data = serial_read();
    if (data != SERIAL_NO_DATA)
    {
        printf("read data: %d\n", data);
    }
    else
    {
        printf("no data available\n");
        // printf("cached data: %d\n", serial_get_rx_buffer_count());
        // printf("available data: %d\n", serial_get_rx_buffer_available());
        TEST_ASSERT_EQUAL(0, serial_get_rx_buffer_count());
        TEST_ASSERT_EQUAL((RX_BUFFER_SIZE * 2), serial_get_rx_buffer_available());
    }

    // 效果
    // 输入数据
    // read data: 1
    // no data available
    // cached data: 0
    // available data: 256
}

void app_main()
{
    serial_init();
    while (1)
    {
        test_read();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}