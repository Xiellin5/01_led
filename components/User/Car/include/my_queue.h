#pragma once
#include <stdbool.h>
#include "espdet_run.h"
#include "FreeRTOS/FreeRTOS.h"

typedef struct
{
    const uint8_t *start;
    const uint8_t *end;
} jpeg_msg_t; // 摄像头数据格式

void jpeg_queue_init(bool is_rst);
bool jpeg_queue_send(const uint8_t *start, const uint8_t *end);
bool jpeg_queue_receive(const uint8_t **start, const uint8_t **end, TickType_t timeout);

typedef human_fall_result_t ai_result_t; // 重命名
void ai_result_queue_init(bool is_rst);
bool ai_result_queue_send(ai_result_t result);
bool ai_result_queue_receive(ai_result_t *result, TickType_t timeout);
/*-----------------------------------------宏定义区------------------------------------------ */
#define CAMERA_JPEG_QUEUE_SIZE (5)
#define AI_RESULT_QUEUE_SIZE (5)
/*------------------------------------------------------------------------------------------ */