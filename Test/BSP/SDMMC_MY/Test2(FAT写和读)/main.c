#include "freertos/FreeRTOS.h"
#include "i2s.h"
#include "nvs_flash.h"
#include "sdmmc.h"
#include "led.h"

void write_hello_file(void)
{
    const char *file_path = MOUNT_POINT "/hello.txt";

    // 打开文件（不存在则创建，存在则覆盖）
    FILE *f = fopen(file_path, "w");
    if (f == NULL)
    {
        ESP_LOGE("sdmmc", "Failed to open file for writing");
        return;
    }

    // 写入内容
    fprintf(f, "hello,world!\n");

    // 关闭文件
    fclose(f);
    ESP_LOGI("sdmmc", "File written successfully: %s", file_path);
}

void read_hello_file(void)
{
    const char *file_path = MOUNT_POINT "/hello.txt";

    // 打开文件（只读模式）
    FILE *f = fopen(file_path, "r");
    if (f == NULL)
    {
        ESP_LOGE("sdmmc", "Failed to open file for reading");
        return;
    }

    // 读取内容并打印
    char line[64];
    while (fgets(line, sizeof(line), f) != NULL)
    {
        // 去掉末尾换行符（可选）
        ESP_LOGI("sdmmc", "Read from file: '%s'", line);
    }

    // 关闭文件
    fclose(f);
}

// 效果：挂载SD卡，创建文件，写入内容，读取内容，LED闪烁
void app_main(void)
{
    esp_err_t ret;
    uint32_t size = 0;

    ret = nvs_flash_init(); /* 初始化NVS */
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    led_init(); /* LED初始化 */
    // lcd_init(); /* LCD屏初始化 */

    // lcd_show_string(30, 50, 200, 16, 16, "ESP32-P4", RED);
    // lcd_show_string(30, 70, 200, 16, 16, "SD TEST", RED);
    // lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

    while (sdmmc_init()) /* 检测不到SD卡 */
    {
        // lcd_show_string(30, 110, 200, 16, 16, "SD Card Error!", RED);
        vTaskDelay(pdMS_TO_TICKS(200));
        printf("SD Card Not Found!\n");
        // lcd_fill(30, 110, 239, 126, WHITE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // lcd_show_string(30, 110, 200, 16, 16, "SD Card OK!", RED);
    // lcd_show_string(30, 130, 200, 16, 16, "Total:      MB", RED);
    printf("SD Card OK!\n");
    size = ((uint64_t)card->csd.capacity) * card->csd.sector_size / (1024 * 1024);
    // lcd_show_num(80, 130, size, 5, 16, BLUE);
    printf("Total: %ld MB\n", size);

    write_hello_file();
    read_hello_file();
    while (1)
    {
        LED_TOGGLE();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}