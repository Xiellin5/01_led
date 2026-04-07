#include "esp8266.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "FreeRTOS/FreeRTOS.h"
#include "string.h"

char *TAG = "esp8266";

WIFI wifi = {0};
uint8_t at_cmd[512];

static esp_err_t ESP8266_USART_Init(void)
{
    static uint8_t is_init = 0;
    if (is_init)
    {
        return ESP_OK; // 已经初始化过了，直接返回成功
    }
    memset(wifi.rxbuff, 0, ESP8266_BUF_SIZE);
    memset(wifi.wifi_ip, 0, ESP8266_BUF_SIZE);
    memset(at_cmd, 0, sizeof(at_cmd));
    uart_config_t uart_config = {
        .baud_rate = ESP8266_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .parity = UART_PARITY_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
        .stop_bits = UART_STOP_BITS_1,
    };

    ESP_ERROR_CHECK(uart_param_config(ESP8266_USART_UX, &uart_config)); // init
    // Set UART pins
    ESP_ERROR_CHECK(uart_set_pin(ESP8266_USART_UX, ESP8266_TX_PIN, ESP8266_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // Install UART driver for interrupt-driven reads and writes
    ESP_ERROR_CHECK(uart_driver_install(ESP8266_USART_UX, ESP8266_RX_BUF_SIZE, ESP8266_TX_BUF_SIZE, 0, NULL, 0));

    is_init = 1;
    return ESP_OK;
}

/**
 * @brief send one byte to ESP8266
 *
 * @param data 待发送字节
 */
static void ESP8266_SendByte(uint8_t data)
{
    uart_write_bytes(ESP8266_USART_UX, &data, 1);
}
/**
 * @brief send string to ESP8266
 *
 * @param str 待发送字符串（以\0结尾）
 */
void ESP8266_SendString(uint8_t *str)
{
    for (uint16_t i = 0; i < strlen((char *)str); i++)
    {
        ESP8266_SendByte(str[i]);
    }
}

/**
 * @brief read string from ESP8266
 *
 * @param data store read data
 * @param length store read data length
 */
void ESP8266_ReadString(uint8_t *data, uint16_t *length)
{
    ESP_ERROR_CHECK(uart_get_buffered_data_len(ESP8266_USART_UX, (size_t *)length));
    if (length == 0)
    {
        return;
    }
    int ret = uart_read_bytes(ESP8266_USART_UX, (void *)data, (uint32_t)*length, pdMS_TO_TICKS(10));
    if (ret == -1)
    {
        ESP_LOGE(TAG, "uart read fail\n");
        return;
    }
}

/**
 * @brief ESP8266发送测试
 *
 */
void ESP8266_Send_test(void)
{
    ESP8266_SendString((uint8_t *)"123\r\n");
}

/**
 * @brief esp8266初始化
 *
 */
void ESP8266_Init(void)
{
    esp_err_t ret;
    ret = ESP8266_USART_Init();
    ESP_ERROR_CHECK(ret);
}

/**
 * @brief esp8266发送AT指令
 *
 * @param cmd 待发送命令
 * @param expect 命令期望的结果
 * @param outtime 等待回应的时间（单位ms）
 * @return esp_err_t 错误码
 */
esp_err_t ESP8266_Send_Cmd(uint8_t *cmd, uint8_t *expect, uint32_t outtime)
{
    uint8_t data = 0;
    uint16_t length = 0;
    // 发送AT指令之前要清空

    memset(wifi.rxbuff, 0, ESP8266_BUF_SIZE);

    ESP8266_SendString(cmd);                  // 发送AT指令
    vTaskDelay(pdMS_TO_TICKS(outtime));       // 等待回应
    ESP8266_ReadString(wifi.rxbuff, &length); // 读取数据
    if (length > 0)
    {
        ESP_LOGI(TAG, "recv data: %s", wifi.rxbuff); // 打印回应数据
        if (expect == NULL || *expect == '\0')
        {
            data = 1;
        }
        else if (strstr((char *)wifi.rxbuff, (char *)expect))
        {
            data = 1;
        }
    }

    // strstr();
    // while (outtime)
    // {
    //     if (wifi.rxover == 1)
    //     {
    //         if (strstr((char *)wifi.rxbuff, (char *)Rcmd) != NULL)
    //         {
    //             data = 1;
    //             break;
    //         }
    //     }
    //     outtime--;
    //     Delay_ms(1);
    // }
    return (data == 0)
               ? ESP_FAIL
               : ESP_OK;
}

/**
 * @brief esp8266测试
 *
 * @return esp_err_t 错误码
 */
esp_err_t ESP8266_AT_Test(void)
{
    esp_err_t ret;
    ret = ESP8266_Send_Cmd((uint8_t *)"AT\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS);
    if (ret == ESP_OK)
    {
        printf("AT Test success!\r\n");
    }
    else
    {
        printf("AT Test send failed!\r\n");
    }
    return ret;
}

/**
 * @brief 连接网络
 *
 * @param ssid 网络名
 * @param password 网络密码
 * @param trytimes 可以尝试的次数，如果连接失败会再次尝试，直到达到trytimes次数
 * @return esp_err_t 错误码
 */
esp_err_t ESP8266_CWJAP(uint8_t *ssid, uint8_t *password, uint8_t trytimes)
{

    // 清空缓冲区
    memset(at_cmd, 0, sizeof(at_cmd));

    // 🔥 关键：自动拼接成：AT+CWJAP="ssid","password"\r\n
    sprintf((char *)at_cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", (char *)ssid, (char *)password);

    // 发送指令并等待 OK
    esp_err_t ret;

    for (int i = 0; i < trytimes; i++)
    {
        ret = ESP8266_Send_Cmd(at_cmd, (uint8_t *)"OK", ESP8266_WAIT_MS * 30);
        if (ret == ESP_OK)
        {
            break;
        }
        else
        {
            ESP_LOGE(TAG, "CWJAP %d failed, try again!", i);
        }
    }

    return ret;
}

/**
 * @brief 建立UDP连接
 *
 * @note 使用完这个函数后，还需要手动在PC端用AT指令让另外一个esp8266跟mcu的esp8266建立udp连接
 *       否则有可能无法实现数据传输
 * @param remote_ip 远端的ip地址
 * @param remote_port 远端ip地址的端口
 * @param client_port 本地端口
 * @param trytimes 可以尝试的次数，如果udp连接失败会再次尝试，直到达到trytimes次数
 * @return esp_err_t
 */
esp_err_t ESP8266_Build_UDP_Connect(uint8_t *remote_ip, uint16_t remote_port, uint16_t client_port, uint8_t trytimes)
{
    memset(at_cmd, 0, sizeof(at_cmd));

    // 🔥 关键：自动拼接成：AT+CWJAP="ssid","password"\r\n
    sprintf((char *)at_cmd, "AT+CIPSTART=\"UDP\",\"%s\",%d,%d,0\r\n",
            (char *)remote_ip, // 字符串IP %s
            remote_port,       // 数字端口 %d
            client_port);      // 数字端口 %d
    esp_err_t ret = ESP_FAIL;
    for (int i = 0; i < trytimes; i++)
    {
        ret = ESP8266_Send_Cmd(at_cmd, (uint8_t *)"OK", ESP8266_WAIT_MS * 30);
        if (ret == ESP_OK)
        {
            break;
        }
        else
        {
            ESP_LOGE(TAG, "Build UDP connect %d failed, try again!", i);
        }
    }

    return ret;
}

/**
 * @brief 完整建立UDP透传
 *
 * @param trytimes 可以尝试的次数
 * @return esp_err_t 错误码
 */
esp_err_t ESP8266_UDP_PS(uint8_t trytimes)
{
    esp_err_t ret;

    // ret = ESP8266_AT();
    ret = ESP8266_Send_Cmd((uint8_t *)"AT\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "AT failed!");
        return ret;
    }
    ret = ESP8266_Send_Cmd((uint8_t *)"AT+RST\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "AT+RST failed!");
        return ret;
    }
    ret = ESP8266_Send_Cmd((uint8_t *)"AT+CWMODE=3\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "AT+CWMODE=3 failed!");
        return ret;
    }
    ret = ESP8266_CWJAP((uint8_t *)WIFI_SSID, (uint8_t *)WIFI_PASSWORD, trytimes);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "CWJAP failed!");
        return ret;
    }
    // ret = ESP8266_CIFSR();
    ret = ESP8266_Send_Cmd((uint8_t *)"AT+CIFSR\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS * 100);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "AT+CIFSR failed!");
        return ret;
    }
    int len = strlen((char *)wifi.rxbuff);
    // 复制数据
    memcpy(wifi.wifi_ip, wifi.rxbuff, len);
    // 手动添加字符串结束符
    wifi.wifi_ip[len] = '\0'; // 记录mcu的esp8266的ip有关信息

    ret = ESP8266_Build_UDP_Connect((uint8_t *)WIFI_REMOTE_IP, WIFI_REMOTE_PORT, WIFI_LOCATE_PORT, trytimes);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Build UDP connect failed!");
        return ret;
    }
    ret = ESP8266_Send_Cmd((uint8_t *)"AT+CIPMUX=0\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "AT+CIPMUX=0 failed!");
        return ret;
    }
    ret = ESP8266_Send_Cmd((uint8_t *)"AT+CIPMODE=1\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "AT+CIPMODE=0 failed!");
        return ret;
    }

    ESP_LOGI(TAG, "UDP PS build success! Now you use Start_CIPSend to start send data by PS!");
    return ESP_OK;
}

/**
 * @brief 开启发送数据
 *
 * @return esp_err_t 错误码
 */
esp_err_t ESP8266_Start_CIPSend(void)
{
    esp_err_t ret;
    ret = ESP8266_Send_Cmd((uint8_t *)"AT+CIPSEND\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS);
    return ret;
}

/**
 * @brief 停止发送数据
 *
 * @return esp_err_t 错误码
 */
esp_err_t ESP8266_Stop_CIPSend(void)
{
    esp_err_t ret;
    ESP8266_SendString((uint8_t *)"+++");
    vTaskDelay(pdMS_TO_TICKS(ESP8266_WAIT_MS));
    ret = ESP8266_Send_Cmd((uint8_t *)"AT\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS);
    return ret;
}

/**
 * @brief 结束UDP连接
 *
 * @return esp_err_t 错误码
 */
esp_err_t ESP8266_Stop_UDP_Connect(void)
{
    esp_err_t ret;
    ret = ESP8266_Send_Cmd((uint8_t *)"AT+CIPCLOSE\r\n", (uint8_t *)"OK", ESP8266_WAIT_MS);
    return ret;
}

/**
 * @brief udp透传测试
 *
 * @return esp_err_t 错误码
 */
esp_err_t ESP8266_UDP_PS_Test(void)
{
    esp_err_t ret;
    ret = ESP8266_UDP_PS(10);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "UDP PS build failed!");
        return ret;
    }
    ESP_LOGI(TAG, "UDP PS build success!");
    printf("本地esp8266的ip相关信息:%s\n", (char *)wifi.wifi_ip);
    ret = ESP8266_Start_CIPSend(); // start send data
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Start CIPSEND failed!");
        return ret;
    }
    ESP_LOGI(TAG, "Start CIPSEND success!");
    for (int i = 0; i < 10; i++)
    {
        ESP8266_SendString((uint8_t *)"Hello,world!\r\n");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    ret = ESP8266_Stop_CIPSend(); // stop send data
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Stop CIPSEND failed!");
        return ret;
    }
    ESP_LOGI(TAG, "Stop CIPSEND success!");

    ret = ESP8266_Stop_UDP_Connect(); // stop udp connect
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Stop UDP connect failed!");
        return ret;
    }
    ESP_LOGI(TAG, "Stop UDP connect success!");

    ESP_LOGI(TAG, "UDP PS test success!");
    return ESP_OK;
}