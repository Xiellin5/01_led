#ifndef __MPUIIC_XIAOHEI_H
#define __MPUIIC_XIAOHEI_H
#include "MPU6050_xiaohei_need.h"

#define I2C_MASTER_SCL_IO 26      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 25      /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

// // IO��������
// #define MPU_SDA_IN()              \
//     {                             \
//         GPIOB->CRH &= 0XFFFF0FFF; \
//         GPIOB->CRH |= 8 << 12;    \
//     }
// #define MPU_SDA_OUT()             \
//     {                             \
//         GPIOB->CRH &= 0XFFFF0FFF; \
//         GPIOB->CRH |= 3 << 12;    \
//     }

// IO��������
// #define MPU_IIC_SCL PBout(10) // SCL
// #define MPU_IIC_SDA PBout(11) // SDA
// #define MPU_READ_SDA PBin(11) // ����SDA

// IIC���в�������
// void MPU_IIC_Delay(void);                // MPU IIC��ʱ���� 延时
void MPU_IIC_Init(void); // ��ʼ��IIC��IO�� 初始化
// void MPU_IIC_Start(void);                // ����IIC��ʼ�ź� 开启
// void MPU_IIC_Stop(void);                 // ����IICֹͣ�ź� 中值
void MPU_IIC_Send_Byte(u8 txd);          // IIC����һ���ֽ� 发送单字节
u8 MPU_IIC_Read_Byte(unsigned char ack); // IIC��ȡһ���ֽ� 读取
// u8 MPU_IIC_Wait_Ack(void);               // IIC�ȴ�ACK�ź� 等待ACK
// void MPU_IIC_Ack(void);                  // IIC����ACK�ź� 回复ACK
// void MPU_IIC_NAck(void);                 // IIC������ACK�ź� 回复NACK

void IMPU_IC_Write_One_Byte(u8 daddr, u8 addr, u8 data); // 指定地址发送一个字节
u8 MPU_IIC_Read_One_Byte(u8 daddr, u8 addr);             // 指定地址读取一个字节
#endif
