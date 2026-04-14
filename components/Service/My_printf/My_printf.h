#pragma once
#include "stdbool.h"
#include "stdint.h"

typedef struct printer_t printer_t;
struct printer_t
{
    void (*init)(void);
    void (*print_string)(uint8_t *str);
    bool is_registered;
};

void print_register(void (*init)(void), void (*print_string)(uint8_t *str));
void print_FireWater(int count, ...);
