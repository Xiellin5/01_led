#include "mpuiic_xiaohei.h"
#include "MPU6050_xiaohei_need.h"
static const char *TAG = "mpu6050 test";
static mpu6050_handle_t mpu6050 = NULL;

/**
 * @brief i2c master initialization
 */
static void i2c_bus_init(void)
{
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

/**
 * @brief i2c master initialization
 */
static void i2c_sensor_mpu6050_init(void)
{
	esp_err_t ret;

	i2c_bus_init();
	mpu6050 = mpu6050_create(I2C_MASTER_NUM, MPU6050_I2C_ADDRESS);
	TEST_ASSERT_NOT_NULL_MESSAGE(mpu6050, "MPU6050 create returned NULL");

	ret = mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS);
	TEST_ASSERT_EQUAL(ESP_OK, ret);

	ret = mpu6050_wake_up(mpu6050);
	TEST_ASSERT_EQUAL(ESP_OK, ret);
}

// // MPU IIC 延时函数
// void MPU_IIC_Delay(void)
// {
// 	Delay_us(2);
// }

// 初始化IIC
void MPU_IIC_Init(void)
{
	i2c_bus_init();
	i2c_sensor_mpu6050_init();
}
// 产生IIC起始信号
// void MPU_IIC_Start(void)
// {
// 	MPU_SDA_OUT(); // sda线输出
// 	MPU_IIC_SDA = 1;
// 	MPU_IIC_SCL = 1;
// 	MPU_IIC_Delay();
// 	MPU_IIC_SDA = 0; // START:when CLK is high,DATA change form high to low
// 	MPU_IIC_Delay();
// 	MPU_IIC_SCL = 0; // 钳住I2C总线，准备发送或接收数据
// }
// // 产生IIC停止信号
// void MPU_IIC_Stop(void)
// {
// 	MPU_SDA_OUT(); // sda线输出
// 	MPU_IIC_SCL = 0;
// 	MPU_IIC_SDA = 0; // STOP:when CLK is high DATA change form low to high
// 	MPU_IIC_Delay();
// 	MPU_IIC_SCL = 1;
// 	MPU_IIC_SDA = 1; // 发送I2C总线结束信号
// 	MPU_IIC_Delay();
// }
// // 等待应答信号到来
// // 返回值：1，接收应答失败
// //         0，接收应答成功
// u8 MPU_IIC_Wait_Ack(void)
// {
// 	u8 ucErrTime = 0;
// 	MPU_SDA_IN(); // SDA设置为输入
// 	MPU_IIC_SDA = 1;
// 	MPU_IIC_Delay();
// 	MPU_IIC_SCL = 1;
// 	MPU_IIC_Delay();
// 	while (MPU_READ_SDA)
// 	{
// 		ucErrTime++;
// 		if (ucErrTime > 250)
// 		{
// 			MPU_IIC_Stop();
// 			return 1;
// 		}
// 	}
// 	MPU_IIC_SCL = 0; // 时钟输出0
// 	return 0;
// }
// // 产生ACK应答
// void MPU_IIC_Ack(void)
// {
// 	MPU_IIC_SCL = 0;
// 	MPU_SDA_OUT();
// 	MPU_IIC_SDA = 0;
// 	MPU_IIC_Delay();
// 	MPU_IIC_SCL = 1;
// 	MPU_IIC_Delay();
// 	MPU_IIC_SCL = 0;
// }
// // 不产生ACK应答
// void MPU_IIC_NAck(void)
// {
// 	MPU_IIC_SCL = 0;
// 	MPU_SDA_OUT();
// 	MPU_IIC_SDA = 1;
// 	MPU_IIC_Delay();
// 	MPU_IIC_SCL = 1;
// 	MPU_IIC_Delay();
// 	MPU_IIC_SCL = 0;
// }
// IIC发送一个字节
// 返回从机有无应答
// 1，有应答
// 0，无应答
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

// void MPU_IIC_Send_Byte(u8 txd)
// {
// 	mpu6050_write(mpu6050,);
// }
// 读1个字节，ack=1时，发送ACK，ack=0，发送nACK
// u8 MPU_IIC_Read_Byte(unsigned char ack)
// {
// 	unsigned char i, receive = 0;
// 	MPU_SDA_IN(); // SDA设置为输入
// 	for (i = 0; i < 8; i++)
// 	{
// 		MPU_IIC_SCL = 0;
// 		MPU_IIC_Delay();
// 		MPU_IIC_SCL = 1;
// 		receive <<= 1;
// 		if (MPU_READ_SDA)
// 			receive++;
// 		MPU_IIC_Delay();
// 	}
// 	if (!ack)
// 		MPU_IIC_NAck(); // 发送nACK
// 	else
// 		MPU_IIC_Ack(); // 发送ACK
// 	return receive;
// }
