#include "mpu6050_xiaohei.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

mpu6050_handle_t mpu6050 = NULL;
/**
 * @brief i2c master initialization
 */
static void i2c_bus_init(void)
{
	static bool Is_init = false;
	if (Is_init)
	{
		return;
	}
	else
	{
		Is_init = true;
	}
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

	esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
	TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "I2C config returned error");

	ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
	TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "I2C install returned error");
}

void MPU_IIC_Init()
{
	i2c_bus_init();
};

typedef struct
{
	i2c_port_t bus;
	gpio_num_t int_pin;
	uint16_t dev_addr;
	uint32_t counter;
	float dt; /*!< delay time between two measurements, dt should be small (ms level) */
	struct timeval *timer;
} mpu6050_dev_t;

static esp_err_t mpu6050_write(mpu6050_handle_t sensor, const uint8_t reg_start_addr, const uint8_t *const data_buf, const uint8_t data_len)
{
	mpu6050_dev_t *sens = (mpu6050_dev_t *)sensor;
	esp_err_t ret;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ret = i2c_master_start(cmd);
	assert(ESP_OK == ret);
	ret = i2c_master_write_byte(cmd, sens->dev_addr | I2C_MASTER_WRITE, true);
	assert(ESP_OK == ret);
	ret = i2c_master_write_byte(cmd, reg_start_addr, true);
	assert(ESP_OK == ret);
	ret = i2c_master_write(cmd, data_buf, data_len, true);
	assert(ESP_OK == ret);
	ret = i2c_master_stop(cmd);
	assert(ESP_OK == ret);
	ret = i2c_master_cmd_begin(sens->bus, cmd, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	return ret;
}

static esp_err_t mpu6050_read(mpu6050_handle_t sensor, const uint8_t reg_start_addr, uint8_t *const data_buf, const uint8_t data_len)
{
	mpu6050_dev_t *sens = (mpu6050_dev_t *)sensor;
	esp_err_t ret;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ret = i2c_master_start(cmd);
	assert(ESP_OK == ret);
	ret = i2c_master_write_byte(cmd, sens->dev_addr | I2C_MASTER_WRITE, true);
	assert(ESP_OK == ret);
	ret = i2c_master_write_byte(cmd, reg_start_addr, true);
	assert(ESP_OK == ret);
	ret = i2c_master_start(cmd);
	assert(ESP_OK == ret);
	ret = i2c_master_write_byte(cmd, sens->dev_addr | I2C_MASTER_READ, true);
	assert(ESP_OK == ret);
	ret = i2c_master_read(cmd, data_buf, data_len, I2C_MASTER_LAST_NACK);
	assert(ESP_OK == ret);
	ret = i2c_master_stop(cmd);
	assert(ESP_OK == ret);
	ret = i2c_master_cmd_begin(sens->bus, cmd, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	return ret;
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_sensor_mpu6050_init(void)
{
	static bool Is_init = false;
	if (Is_init)
	{
		return ESP_OK;
	}
	else
	{
		Is_init = true;
	}
	esp_err_t ret;
	uint8_t res;

	MPU_IIC_Init();
	mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS);
	TEST_ASSERT_NOT_NULL_MESSAGE(mpu6050, "MPU6050 create returned NULL");
	printf("yes\n");
	res = MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X80); // 复位MPU6050
	TEST_ASSERT_EQUAL(0, res);
	Delay_ms(100);
	res = MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X00); // 唤醒MPU6050
	TEST_ASSERT_EQUAL(0, res);

	ret = MPU_Set_Gyro_Fsr(mpu6050, 3); // 陀螺仪传感器,±2000dps
	TEST_ASSERT_EQUAL(ESP_OK, ret);
	ret = MPU_Set_Accel_Fsr(mpu6050, 3); // 加速度传感器,±16g
	TEST_ASSERT_EQUAL(ESP_OK, ret);
	ret = MPU_Set_Rate(mpu6050, 50); // 设置采样率50Hz
	TEST_ASSERT_EQUAL(ESP_OK, ret);

	res = MPU_Write_Byte(MPU_INT_EN_REG, 0X00); // 关闭所有中断
	TEST_ASSERT_EQUAL(0, res);
	res = MPU_Write_Byte(MPU_USER_CTRL_REG, 0X00); // I2C主模式关闭
	TEST_ASSERT_EQUAL(0, res);
	res = MPU_Write_Byte(MPU_FIFO_EN_REG, 0X00); // 关闭FIFO
	TEST_ASSERT_EQUAL(0, res);
	res = MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X80); // INT引脚低电平有效
	TEST_ASSERT_EQUAL(0, res);

	ret = mpu6050_read(mpu6050, MPU_DEVICE_ID_REG, &res, 1);
	TEST_ASSERT_EQUAL(ESP_OK, ret);
	if (res == MPU_ADDR) // 器件ID正确
	{
		res = MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X01); // 设置CLKSEL,PLL X轴为参考
		TEST_ASSERT_EQUAL(0, res);
		res = MPU_Write_Byte(MPU_PWR_MGMT2_REG, 0X00); // 加速度与陀螺仪都工作
		TEST_ASSERT_EQUAL(0, res);
		MPU_Set_Rate(mpu6050, 50); // 设置采样率为50Hz
	}
	else
	{
		printf("MPU6050 init error:ID:%02X", res);
		return ESP_OK;
	}

	Delay_ms(50);
	return ESP_FAIL;
}

// 函数功能：初始化DMP并显示错误原因
// 作者：小黑
// 创建时间：2021/5/5
void DMP_Init(void)
{
	int i = 1;
	int dmp_error;						 // 记录dmp错误原因
	while ((dmp_error = mpu_dmp_init())) // DMP初始化（此处报warning不用管）
	{

		printf("trytimes:%d, DMP init error:%d", i++, dmp_error);
		Delay_ms(200);
	}
	Delay_ms(1000);
}

esp_err_t MPU_Init(void)
{
	return i2c_sensor_mpu6050_init();
}
// 设置MPU6050陀螺仪传感器满量程范围
// fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
// 返回值:0,设置成功
//     其他,设置失败
esp_err_t MPU_Set_Gyro_Fsr(mpu6050_handle_t sensor, uint8_t fsr)
{
	uint8_t data = fsr << 3;
	return mpu6050_write(sensor, MPU_GYRO_CFG_REG, &data, 1); // 设置陀螺仪满量程范围
}
// 设置MPU6050加速度传感器满量程范围
// fsr:0,±2g;1,±4g;2,±8g;3,±16g
// 返回值:0,设置成功
//     其他,设置失败
esp_err_t MPU_Set_Accel_Fsr(mpu6050_handle_t sensor, uint8_t fsr)
{
	uint8_t data = fsr << 3;
	return mpu6050_write(sensor, MPU_ACCEL_CFG_REG, &data, 1); // 设置加速度传感器满量程范围
}
// 设置MPU6050的数字低通滤波器
// lpf:数字低通滤波频率(Hz)
// 返回值:0,设置成功
//     其他,设置失败
esp_err_t MPU_Set_LPF(mpu6050_handle_t sensor, uint16_t lpf)
{
	uint8_t data = 0;
	if (lpf >= 188)
		data = 1;
	else if (lpf >= 98)
		data = 2;
	else if (lpf >= 42)
		data = 3;
	else if (lpf >= 20)
		data = 4;
	else if (lpf >= 10)
		data = 5;
	else
		data = 6;
	return mpu6050_write(sensor, MPU_CFG_REG, &data, 1); // 设置数字低通滤波器
}
// 设置MPU6050的采样率(假定Fs=1KHz)
// rate:4~1000(Hz)
// 返回值:0,设置成功
//     其他,设置失败
esp_err_t MPU_Set_Rate(mpu6050_handle_t sensor, uint16_t rate)
{
	uint8_t data;
	if (rate > 1000)
		rate = 1000;
	if (rate < 4)
		rate = 4;
	data = 1000 / rate - 1;
	data = mpu6050_write(sensor, MPU_SAMPLE_RATE_REG, &data, 1); // 设置数字低通滤波器
	return MPU_Set_LPF(sensor, rate / 2);
}

// 得到温度值
// 返回值:温度值(扩大了100倍)
short MPU_Get_Temperature(mpu6050_handle_t sensor)
{
	esp_err_t ret;
	uint8_t buf[2];
	short raw;
	float temp;
	ret = mpu6050_read(sensor, MPU_TEMP_OUTH_REG, buf, 2);
	TEST_ASSERT_EQUAL(ESP_OK, ret);
	raw = ((uint16_t)buf[0] << 8) | buf[1];
	temp = 36.53 + ((double)raw) / 340;
	return temp * 100;
}
// 得到陀螺仪值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
// 返回值:0,成功
//     其他,错误代码
esp_err_t MPU_Get_Gyroscope(mpu6050_handle_t sensor, short *gx, short *gy, short *gz)
{
	uint8_t buf[6];
	esp_err_t ret;
	ret = mpu6050_read(sensor, MPU_GYRO_XOUTH_REG, buf, 6);
	if (ret == ESP_OK)
	{
		*gx = ((uint16_t)buf[0] << 8) | buf[1];
		*gy = ((uint16_t)buf[2] << 8) | buf[3];
		*gz = ((uint16_t)buf[4] << 8) | buf[5];
	}
	return ret;
}
// 得到加速度值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
// 返回值:0,成功
//     其他,错误代码
esp_err_t MPU_Get_Accelerometer(mpu6050_handle_t sensor, short *ax, short *ay, short *az)
{
	uint8_t buf[6];
	esp_err_t ret;
	ret = mpu6050_read(sensor, MPU_ACCEL_XOUTH_REG, buf, 6);
	if (ret == ESP_OK)
	{
		*ax = ((uint16_t)buf[0] << 8) | buf[1];
		*ay = ((uint16_t)buf[2] << 8) | buf[3];
		*az = ((uint16_t)buf[4] << 8) | buf[5];
	}
	return ret;
}
// IIC连续写
// addr:器件地址
// reg:寄存器地址
// len:写入长度
// buf:数据区
// 返回值:0,正常
//     其他,错误代码
uint8_t MPU_Write_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	esp_err_t ret;
	ret = mpu6050_write(mpu6050, reg, buf, len);
	if (ret == ESP_OK)
	{
		return 0;
	}
	else
		return 1;
}
// IIC连续读
// addr:器件地址
// reg:要读取的寄存器地址
// len:要读取的长度
// buf:读取到的数据存储区
// 返回值:0,正常
//     其他,错误代码
uint8_t MPU_Read_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
	esp_err_t ret;
	ret = mpu6050_read(mpu6050, reg, buf, len);
	if (ret == ESP_OK)
	{
		return 0;
	}
	else
		return 1;
}
// IIC写一个字节
// reg:寄存器地址
// data:数据
// 返回值:0,正常
//     其他,错误代码
uint8_t MPU_Write_Byte(uint8_t reg, uint8_t data)
{
	esp_err_t ret;
	ret = mpu6050_write(mpu6050, reg, &data, 1);
	if (ret == ESP_OK)
	{
		return 0;
	}
	else
		return 1;
}
// IIC读一个字节
// reg:寄存器地址
// 返回值:读到的数据
uint8_t MPU_Read_Byte(uint8_t reg)
{
	esp_err_t ret;
	uint8_t res;
	ret = mpu6050_read(mpu6050, reg, &res, 1);
	TEST_ASSERT_EQUAL(ret, ESP_OK);
	return res;
}
