#ifndef __CAMERA_TASK_H__
#define __CAMERA_TASK_H__

/**
 * @brief 摄像头接收任务 (User Layer)
 * 负责调用 Middleware 层的 SPI 从机驱动，申请 DMA 内存，
 * 并循环等待接收来自主机的图像数据。
 *
 * @param pvParameters 传给任务的参数（通常为 NULL）
 */
void camera_task(void *pvParameters);

#endif /* __CAMERA_TASK_H__ */