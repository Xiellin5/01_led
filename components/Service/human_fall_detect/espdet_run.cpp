#include "espdet_run.h"
#include "espdet_detect.hpp"
#include "dl_image_jpeg.hpp"
#include "esp_log.h"
#include <stdint.h>

static ESPDetDetect *detect = nullptr;
static const char *TAG = "human_fall_detect";

/**
 * @brief 初始化人体检测模型
 *
 * @param if_info 是否打印模型信息
 */
extern "C" void init_human_fall_detect(bool if_info)
{
    ESP_LOGI(TAG, "init_human_fall_detect");
#if CONFIG_ESPDET_DETECT_MODEL_IN_SDCARD
    ESP_ERROR_CHECK(bsp_sdcard_mount());
#endif
    if (detect == nullptr)
    {
        detect = new ESPDetDetect();
    }
    if (if_info) // 要打印模型具体信息
    {
        detect->get_raw_model()->profile_memory();
        detect->get_raw_model()->profile_module(true);
    }
}

/**
 * @brief 运行人体检测模型
 *
 * @param jpg_start JPEG图片数据起始地址
 * @param jpg_end JPEG图片数据结束地址
 * @param callback 检测结果回调函数
 * @param user_data 用户数据指针，将原样传入回调函数
 */
extern "C" void run_human_fall_detect(const uint8_t *jpg_start, const uint8_t *jpg_end,
                                      detect_callback_t callback, void *user_data)
{
    if (detect == nullptr)
    {
        init_human_fall_detect(false);
    }
    if (detect == nullptr)
    {
        ESP_LOGE(TAG, "Detector not initialized");
        return;
    }
    dl::image::jpeg_img_t jpeg_img = {.data = (void *)jpg_start, .data_len = (size_t)(jpg_end - jpg_start)};
    auto img = dl::image::sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888);

    auto &detect_results = detect->run(img);
    for (const auto &res : detect_results)
    {
        // ESP_LOGI(TAG,
        //          "[category: %d, score: %f, x1: %d, y1: %d, x2: %d, y2: %d]",
        //          res.category,
        //          res.score,
        //          res.box[0],
        //          res.box[1],
        //          res.box[2],
        //          res.box[3]);
        // 2. 构造 C 结构体并回调
        if (callback != nullptr)
        {
            human_fall_result_t c_res;
            c_res.category = res.category;
            c_res.score = res.score;
            c_res.box[0] = res.box[0];
            c_res.box[1] = res.box[1];
            c_res.box[2] = res.box[2];
            c_res.box[3] = res.box[3];
            callback(&c_res, user_data);
        }
    }
    heap_caps_free(img.data);
}

/**
 * @brief 释放人体检测模型资源
 */
extern "C" void deinit_human_fall_detect(void)
{
    if (detect != nullptr)
    {
        delete detect;
        detect = nullptr;
    }
#if CONFIG_ESPDET_DETECT_MODEL_IN_SDCARD
    bsp_sdcard_unmount();
#endif
}
