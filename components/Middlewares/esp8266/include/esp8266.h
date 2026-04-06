#ifndef _ESP8266_H_
#define _ESP8266_H_
#include "stdint.h"
#include "esp_system.h"

/* 引脚和串口定义 */
#define ESP8266_USART_UX UART_NUM_1
#define ESP8266_TX_PIN 45
#define ESP8266_RX_PIN 46
#define ESP8266_BAUD_RATE 115200
/* 串口接收相关定义 */
#define ESP8266_BUF_SIZE 1024
#define ESP8266_RX_BUF_SIZE ESP8266_BUF_SIZE * 2
#define ESP8266_TX_BUF_SIZE ESP8266_BUF_SIZE * 2
/* 网络相关定义 */
#define WIFI_SSID "www"
#define WIFI_PASSWORD "www123456"
#define WIFI_REMOTE_IP "192.168.227.167"
#define WIFI_REMOTE_PORT 8081
#define WIFI_LOCATE_PORT 8080
/* 等待响应时间 */
#define ESP8266_WAIT_MS 100

typedef struct
{
    uint8_t rxbuff[ESP8266_BUF_SIZE];

    uint8_t wifi_ip[ESP8266_BUF_SIZE];
} WIFI;

extern WIFI wifi;

void ESP8266_Init(void);
void ESP8266_SendString(uint8_t *str);
void ESP8266_ReadString(uint8_t *data, uint16_t *length);
esp_err_t ESP8266_Send_Cmd(uint8_t *cmd, uint8_t *expect, uint32_t outtime);

esp_err_t ESP8266_UDP_PS(uint8_t trytimes);
esp_err_t ESP8266_Stop_UDP_Connect(void);

esp_err_t ESP8266_Start_CIPSend(void);
esp_err_t ESP8266_Stop_CIPSend(void);

esp_err_t ESP8266_AT_Test(void);
void ESP8266_Send_test(void);
esp_err_t ESP8266_UDP_PS_Test(void);

#endif /* _ESP8266_H_ */
