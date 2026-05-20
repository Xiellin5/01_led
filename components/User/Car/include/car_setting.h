#ifndef __CAR_SETTING_H__
#define __CAR_SETTING_H__

// 电机引脚
#define CAR_L_AIN1_PIN 28
#define CAR_L_AIN2_PIN 29
#define CAR_L_PWMA_PIN 31

#define CAR_R_BIN1_PIN 16
#define CAR_R_BIN2_PIN 17
#define CAR_R_PWMB_PIN 18

// pid参数
#define CAR_KPL 0.5
#define CAR_KIL 0.0
#define CAR_KDL 0.0

#define CAR_KPR 0.5
#define CAR_KIR 0.0
#define CAR_KDR 0.0

// 任务优先级
#define AI_TASK_PRIORITY 19
#define PID_TASK_PRIORITY 18
#define CAMERA_TASK_PRIORITY 18

#endif /* __CAR_SETTING_H__ */
