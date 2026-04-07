#include "pid.h"
/**
 * @file    pid.c
 * @brief   pid控制器实现,可以使用mode参数设置pid的不同特性
 * @author  Zhang
 * @date    2025-03-17
 * @version 1.0
 * @note
 */
static float I_max_limit = 10;
static float Total_max_limit = 100;
static float I_bound = 10;

/**
 * 设置pid参数
 * @param pid pid控制器
 * @param kp proportional gain
 * @param ki integral gain
 * @param kd derivative gain
 */
void pid_set_k(pid_t *pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

/**
 * @brief 设置pid目标值
 *
 * @param pid pid控制器
 * @param target 目标值
 */
void pid_set_target(pid_t *pid, float target)
{
    pid->target = target;
}

/**
 * @brief 设置pid当前值
 *
 * @param pid pid控制器
 * @param current 当前值
 */
void pid_set_current(pid_t *pid, float current)
{
    pid->current = current;
}

/**
 * @brief 设置pid当前值
 *
 * @param pid pid控制器
 * @param i_max_limit 最大积分值
 */
void pid_set_i_max_limit(pid_t *pid, float i_max_limit)
{
    pid->i_max_limit = i_max_limit;
}

/**
 * @brief 设置pid当前值
 *
 * @param pid pid控制器
 * @param total_max_limit 最大输出值
 */
void pid_set_total_max_limit(pid_t *pid, float total_max_limit)
{
    pid->total_max_limit = total_max_limit;
}

/**
 * @brief 设置pid当前值
 *
 * @param pid pid控制器
 * @param i_bound 开启积分输出值的阈值
 */
void pid_set_i_bound(pid_t *pid, float i_bound)
{
    pid->i_bound = i_bound;
}

/**
 * @brief
 *
 * @param pid       pid控制器
 * @return true     成功
 * @return false    失败
 */
bool pid_init(pid_t *pid)
{
    pid->kp = 0.0; // proportional gain
    pid->ki = 0.0; // integral gain
    pid->kd = 0.0; // derivative gain

    pid->p_out = 0.0;     // proportional output
    pid->i_out = 0.0;     // integral output
    pid->d_out = 0.0;     // derivative output
    pid->total_out = 0.0; // total output

    pid->target = 0.0;       // target value
    pid->current = 0.0;      // current value
    pid->last_current = 0.0; // last current value
    pid->e = 0.0;            // error
    pid->last_e = 0.0;       // last error

    pid->i_max_limit = I_max_limit;         // maximum limit of integral term
    pid->total_max_limit = Total_max_limit; // maximum limit of total term
    pid->i_bound = I_bound;                 // integral bound
    return true;
}

// limit integral term
static void pid_limit_i(pid_t *pid)
{
    pid->i_out = pid->i_out > pid->i_max_limit ? pid->i_max_limit : pid->i_out;
};

// limit total term
static void pid_limit_total(pid_t *pid)
{
    pid->total_out = pid->total_out > pid->total_max_limit ? pid->total_max_limit : pid->total_out;
}

// bound integral term
static void pid_bound_i(pid_t *pid)
{
    if (fabs(pid->i_out) > pid->i_bound)
    {
        pid->i_out = 0.0;
    }
}

// 微分先行
static void pid_ahead_d(pid_t *pid)
{
    pid->d_out = -1.0 * pid->kd * (pid->current - pid->last_current);
}

void pid_calc(pid_t *pid)
{
    pid->e = pid->target - pid->current;

    pid->p_out = pid->kp * pid->e;
    pid->i_out += pid->ki * pid->e;
    pid->d_out = pid->kd * (pid->e - pid->last_e);

    // 积分限幅
    if (pid->mode & 0x01)
    {
        pid_limit_i(pid);
    }

    // 积分分离
    if (pid->mode & 0x03)
    {
        pid_bound_i(pid);
    }

    // 微分先行
    if (pid->mode & 0x04)
    {
        pid_ahead_d(pid);
    }

    pid->total_out = pid->p_out + pid->i_out + pid->d_out;

    // 输出限幅
    if (pid->mode & 0x02)
    {
        pid_limit_total(pid);
    }

    pid->last_e = pid->e;
    pid->last_current = pid->current;
};

// 创建不同类型的pid_t
// 八位从低到高分别表示(1:开启，2:不开启):
// 是否开启积分限幅
// 是否开启输出限幅
// 是否开启积分分离
// 是否开启微分先行
void pid_create(pid_t *pid, uint8_t mode)
{
    pid->mode = mode;
};
