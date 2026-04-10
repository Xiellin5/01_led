#ifndef __CAR_H__
#define __CAR_H__

#include "car_setting.h"
#include "stdint.h"
#include "motor.h"
#include "pid.h"

#define ANY_SPEED -999
#define ANY_DISTANCE -999
#define ANY_ANGLE -999

typedef struct car_handle_t car_handle_t;
struct car_handle_t
{
    float line_distance;  // 当前行驶距离
    float circle_angle;   // 当前转弯角度
    int32_t line_speed;   // 当前的速度
    int32_t circle_speed; // 当前的角速度

    float target_line_distance;  // 目标行驶距离
    float target_circle_angle;   // 目标转弯角度
    int32_t target_line_speed;   // 目标速度
    int32_t target_circle_speed; // 目标角速度

    motor_handle_t motor_l; // 左轮
    motor_handle_t motor_r; // 右轮

    pid_t pid_l; // 左轮PID
    pid_t pid_r; // 右轮PID
};
extern car_handle_t car_handle;

#define car_set_motor_l(speed)                       \
    do                                               \
    {                                                \
        int8_t _speed = (speed);                     \
        Motor_SetSpeed(&car_handle.motor_l, _speed); \
    } while (0) // 左轮速度控制

#define car_set_motor_r(speed)                       \
    do                                               \
    {                                                \
        int8_t _speed = (speed);                     \
        Motor_SetSpeed(&car_handle.motor_r, _speed); \
    } while (0) // 右轮速度控制

void car_Init(void);
void car_run_line(int32_t speed, uint32_t distance);
void car_run_circle(int32_t speed, uint32_t angle);
void car_stop(void);

#endif /* __CAR_H__ */
