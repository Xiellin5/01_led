#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    int category;
    float score;
    int box[4];
} human_fall_result_t;

#ifdef __cplusplus
extern "C"
{
#endif
    typedef void (*detect_callback_t)(const human_fall_result_t *result, void *user_data);
    // 声明供 C 调用的函数
    void init_human_fall_detect(bool if_info);
    void run_human_fall_detect(const uint8_t *jpg_start, const uint8_t *jpg_end,
                               detect_callback_t callback, void *user_data);
    void deinit_human_fall_detect(void);
#ifdef __cplusplus
}
#endif