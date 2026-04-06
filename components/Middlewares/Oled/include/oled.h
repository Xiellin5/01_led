#ifndef __OLED_H
#define __OLED_H
#include "stdint.h"
#define OLED_SCL 24
#define OLED_SDA 25
// 以us作为单位，越小则说明oled加载速度越快，但不能为0，且可能较小时部分情况oled显示不出来hh
#define OLED_LOAD_SPEED 1

#define OLED_W_SCL(x) gpio_set_level(OLED_SCL, (x))
#define OLED_W_SDA(x) gpio_set_level(OLED_SDA, (x))
void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

#endif
