#include <stdio.h>
#include "espdet_run.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
extern const uint8_t person_jpg_start[] asm("_binary_person_jpg_start");
extern const uint8_t person_jpg_end[] asm("_binary_person_jpg_end");
human_fall_result_t g_results[10];
int g_result_count = 0;

void print_callback(const human_fall_result_t *res, void *user_data)
{
    // 处理每个检测结果，例如保存到数组、触发GPIO等
    printf("Fall detected: score=%.2f box=[%d,%d,%d,%d]\n",
           res->score, res->box[0], res->box[1], res->box[2], res->box[3]);
}
void my_callback(const human_fall_result_t *res, void *user_data)
{
    if (g_result_count < 10)
    {
        g_results[g_result_count++] = *res; // 浅拷贝（注意内部无动态指针，安全）
    }
}
void test_callback(const human_fall_result_t *res, void *user_data)
{
    return;
}

// 测试ai的C封装
// 结果：第一个回调函数打印结果信息，第二个回调函数记录结果信息，最后在循环里重复跑模型打印结果
void app_main(void)
{
    init_human_fall_detect(false);
    run_human_fall_detect(person_jpg_start, person_jpg_end, print_callback, NULL);
    deinit_human_fall_detect();
    run_human_fall_detect(person_jpg_start, person_jpg_end, my_callback, NULL);
    printf("Results:\n");
    for (int i = 0; i < g_result_count; i++)
    {
        printf("Fall detected: score=%.2f box=[%d,%d,%d,%d]\n",
               g_results[i].score, g_results[i].box[0], g_results[i].box[1], g_results[i].box[2], g_results[i].box[3]);
    }
    while (1)
    {
        run_human_fall_detect(person_jpg_start, person_jpg_end, test_callback, NULL);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}