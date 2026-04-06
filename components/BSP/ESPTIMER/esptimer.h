#ifndef _ESPTIMER_H_
#define _ESPTIMER_H_
#include "esp_system.h"
#include "esp_timer.h"
#include "led.h"

void esptimer_init(uint64_t tps);
#endif /* _ESPTIMER_H_ */
