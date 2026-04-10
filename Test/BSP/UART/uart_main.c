#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "exti.h"
#include "led.h"
#include "uart.h"
void app_main(void)
{
    // esp_err_t ret;
    uint16_t len = 0;
    char data[10] = {0};
    char *a = "LED_ON";
    char *b = "LED_OFF";
    led_init();
    uart_init(115200);

    while (1)
    {
        uart_get_buffered_data_len(USART_UX, (size_t *)&len);
        if (len > 0)
        {
            uart_read_bytes(USART_UX, data, len, 100);
            data[len] = '\0';
            if (strcmp(data, a) == 0)
            {
                LED_ON();
                uart_write_bytes(USART_UX, "LED_ON\r\n", 8);
            }
            else if (strcmp(data, b) == 0)
            {
                LED_OFF();
                uart_write_bytes(USART_UX, "LED_OFF\r\n", 9);
            }
            else
            {
                uart_write_bytes(USART_UX, "Invalid command\r\n", 18);
            }
            memset(data, 0, 10);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
