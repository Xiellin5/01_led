#include "mpu6050_xiaohei.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "FreeRTOS/FreeRTOS.h"

// 注意GND和VCC不要断，断开后得手动BOOT重启
// 下面是一种读取三个角度的方法
// 也可以使用外来库的espressif_mpu6050的函数：mpu6050_complimentory_filter获取角度（只是没有yaw角）
void app_main(void)
{
    float pitch, roll, yaw; // 欧拉角原始数据

    MPU_Init(); // MPU6050初始化
    DMP_Init(); // DMP初始化（DMP用来解算陀螺仪数据）

    // 不断地读取陀螺仪角度并显示
    while (1)
    {
        if (mpu_dmp_get_data(&pitch, &roll, &yaw) == 0) // 得到了欧拉角，存在pitch,roll,yaw里面
        {
            printf("pitch:%.1f°  roll:%.1f°  yaw:%.1f°\r\n", pitch, roll, yaw);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 延时100ms
    }
}
