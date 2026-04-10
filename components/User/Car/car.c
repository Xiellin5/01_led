#include "car.h"

car_handle_t car_handle = {0};
// 初始化
void car_Init(void)
{
    car_handle.line_distance = 0;
    car_handle.circle_angle = 0;
    car_handle.line_speed = 0;
    car_handle.circle_speed = 0;

    car_handle.target_line_distance = 0;
    car_handle.target_circle_angle = 0;
    car_handle.target_line_speed = 0;
    car_handle.target_circle_speed = 0;

    // 初始化电机
    car_handle.motor_l.gpio_in1 = CAR_L_AIN1_PIN;
    car_handle.motor_l.gpio_in2 = CAR_L_AIN2_PIN;
    car_handle.motor_l.gpio_pwmin = CAR_L_PWMA_PIN;
    car_handle.motor_r.gpio_in1 = CAR_R_BIN1_PIN;
    car_handle.motor_r.gpio_in2 = CAR_R_BIN2_PIN;
    car_handle.motor_r.gpio_pwmin = CAR_R_PWMB_PIN;
    // printf("be:l,in1:%d,in2:%d,pwmin:%d\n", car_handle.motor_l.gpio_in1, car_handle.motor_l.gpio_in2, car_handle.motor_l.gpio_pwmin);
    // printf("be:r,in1:%d,in2:%d,pwmin:%d\n", car_handle.motor_r.gpio_in1, car_handle.motor_r.gpio_in2, car_handle.motor_r.gpio_pwmin);
    MOTOR_Init(&car_handle.motor_l);
    MOTOR_Init(&car_handle.motor_r);
    // printf("af:l,in1:%d,in2:%d,pwmin:%d\n", car_handle.motor_l.gpio_in1, car_handle.motor_l.gpio_in2, car_handle.motor_l.gpio_pwmin);
    // printf("af:r,in1:%d,in2:%d,pwmin:%d\n", car_handle.motor_r.gpio_in1, car_handle.motor_r.gpio_in2, car_handle.motor_r.gpio_pwmin);

    // 初始化pid
    pid_create(&car_handle.pid_l, 0);
    pid_create(&car_handle.pid_r, 0);
    pid_init(&car_handle.pid_l);
    pid_init(&car_handle.pid_r);
    pid_set_k(&car_handle.pid_l, CAR_KPL, CAR_KIL, CAR_KDL);
    pid_set_k(&car_handle.pid_r, CAR_KPR, CAR_KIR, CAR_KDR);
}

// 指定速度运行指定的距离
void car_run_line(int32_t speed, uint32_t distance)
{
    if (speed == ANY_SPEED)
    {
        // 初速度为0，最终速度为0，任意速度跑完距离即可-->S型加减速
        return;
    }

    if (distance == ANY_DISTANCE)
    {
        // 维持speed做匀速直线运动即可
        return;
    }

    // 初速度为0，最终速度为0，最大速度为speed（达到则维持）跑完distance

    return;
}
// 指定速度运行指定的角度
void car_run_circle(int32_t speed, uint32_t angle)
{
    if (speed == ANY_SPEED)
    {
        // 初速度为0，最终速度为0，任意速度转完angle即可
        return;
    }

    if (angle == ANY_ANGLE)
    {
        // 维持speed做匀速转弯运动即可
        return;
    }

    // 初速度为0，最终速度为0，最大速度为speed（达到则维持）转完angle即可

    return;
}
// 停止
void car_stop(void)
{
    car_run_line(0, 0);
    car_run_circle(0, 0);
}