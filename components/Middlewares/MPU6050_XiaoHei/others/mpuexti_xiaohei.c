#include "mpuexti_xiaohei.h"
#include "mpu6050_xiaohei.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "mpu6050_xiaohei_need.h"
float pitch, roll, yaw; // 欧拉角原始数据

// 函数功能：初始化接收陀螺仪中断的引脚
void MPU_exti_init()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // 使能复用功能时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	  // PA12
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 设置成上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12); // 将PA12映射到中断线12
	EXTI_InitStructure.EXTI_Line = EXTI_Line12;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // 响应优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// // 外部中断线12服务程序（10ms中断）
// void EXTI15_10_IRQHandler(void)
// {
// 	if (mpu_dmp_get_data(&pitch, &roll, &yaw) == 0)
// 	{
// 		// 此时欧拉角已更新,存在pitch,roll,yaw里面
// 	}

// 	EXTI_ClearITPendingBit(EXTI_Line12); // 清除LINE上的中断标志位
// }
