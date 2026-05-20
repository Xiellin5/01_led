#include "sd_card.h"
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"

static const char *TAG = "SD_CARD";
static int file_count = 0; // 用于生成自动递增的文件名

// 【根据 ESP32-P4 官方文档修正的引脚映射】
// 对应 卡槽 1 (SDMMC_HOST_SLOT_0) IO MUX 固定管脚
#define SD_CLK_PIN  43  // SD1_CCLK_PAD
#define SD_CMD_PIN  44  // SD1_CCMD_PAD
#define SD_D0_PIN   39  // SD1_CDATA0_PAD
#define SD_D1_PIN   40  // SD1_CDATA1_PAD
#define SD_D2_PIN   41  // SD1_CDATA2_PAD
#define SD_D3_PIN   42  // SD1_CDATA3_PAD

#define MOUNT_POINT "/sdcard"

esp_err_t sd_card_init(void) {
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false, // 如果挂载失败，不自动格式化卡
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    sdmmc_card_t *card;
    ESP_LOGI(TAG, "Initializing SD card on ESP32-P4 Slot 0...");

    // 使用 SDMMC 主机控制器接口
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    
    // P4 的默认 Slot 0 通常用于这组固定的高速引脚
    host.slot = SDMMC_HOST_SLOT_0; 
    host.max_freq_khz = SDMMC_FREQ_DEFAULT; // 20MHz，求稳。稳定后可改为 SDMMC_FREQ_HIGHSPEED (40MHz)

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    
    // 启用 4-bit 宽带模式
    slot_config.width = 4; 
    
    // 强制开启内部上拉（虽然官方建议外部上拉，但开启内部双重保险）
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    // 绑定正确的物理引脚
    slot_config.clk = SD_CLK_PIN;
    slot_config.cmd = SD_CMD_PIN;
    slot_config.d0  = SD_D0_PIN;
    slot_config.d1  = SD_D1_PIN;
    slot_config.d2  = SD_D2_PIN;
    slot_config.d3  = SD_D3_PIN;

    // 挂载 FAT 文件系统
    ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card. Error: %s", esp_err_to_name(ret));
        return ret;
    }

    // 打印 SD 卡的基础信息
    ESP_LOGI(TAG, "SD card mounted successfully!");
    sdmmc_card_print_info(stdout, card); 
    
    return ESP_OK;
}

esp_err_t sd_card_save_image(uint8_t *img_data, uint32_t img_len) {
    char filename[64];
    // 生成形如 /sdcard/img_0000.jpg 的文件名
    snprintf(filename, sizeof(filename), "%s/img_%04d.jpg", MOUNT_POINT, file_count);

    ESP_LOGI(TAG, "Saving image %s (%ld bytes)...", filename, img_len);

    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }

    size_t written = fwrite(img_data, 1, img_len, f);
    fclose(f);

    if (written != img_len) {
        // 【已修改】将 %d 修改为 %zu
        ESP_LOGE(TAG, "File write failed! Expected %ld, wrote %zu", img_len, written);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Saved %s successfully", filename);
    file_count++; // 文件名序号递增
    return ESP_OK;
}