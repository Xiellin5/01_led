#pragma once
#include "car_setting.h"
#include "my_queue.h"
#include "ai_task.h"
#include "camera_task.h"
void system_run(void *pvParameters);
void system_create(void);
void system_delete(void);
void system_send_rst(void);
