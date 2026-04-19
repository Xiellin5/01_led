#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX_FRAME  20000   // JPEG一般2~6KB，这里留余量

// 图像数据结构体
typedef struct {
    uint8_t data[MAX_FRAME]; // 存储 JPEG 数据
    uint32_t length;         // 图像实际长度
    bool is_ready;           // 图像准备完成标志位
} Camera_Image_t;

// 函数声明
void Camera_Init(void);
bool Camera_Is_Ready(void);
Camera_Image_t* Camera_Get_Image(void);
void Camera_Clear_Flag(void);

#endif /* _CAMERA_H_ */