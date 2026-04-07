#ifndef MY_PRINTF_H_
#define MY_PRINTF_H_
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

#endif /* MY_PRINTF_H_ */
