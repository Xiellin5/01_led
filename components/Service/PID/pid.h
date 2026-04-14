#pragma once
#include "math.h"
#include "stdbool.h"
#include "stdint.h"

typedef struct pid_t pid_t;

struct pid_t
{
    float kp; // proportional gain
    float ki; // integral gain
    float kd; // derivative gain

    float p_out;     // proportional output
    float i_out;     // integral output
    float d_out;     // derivative output
    float total_out; // total output

    float target;       // target value
    float current;      // current value
    float last_current; // last current value
    float e;            // error
    float last_e;       // last error

    float i_max_limit;     // maximum limit of integral term
    float total_max_limit; // maximum limit of total term
    float i_bound;         // integral bound

    uint8_t mode; // mode of pid controller
};

void pid_create(pid_t *pid, uint8_t mode);
bool pid_init(pid_t *pid);
void pid_calc(pid_t *pid);

void pid_set_current(pid_t *pid, float current);
void pid_set_target(pid_t *pid, float target);
void pid_set_k(pid_t *pid, float kp, float ki, float kd);
void pid_set_i_max_limit(pid_t *pid, float i_max_limit);
void pid_set_total_max_limit(pid_t *pid, float total_max_limit);
void pid_set_i_bound(pid_t *pid, float i_bound);
