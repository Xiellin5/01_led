#include "filter.h"

/*
通用版限幅滤波函数
输入：
  - raw_data: 原始采样数据数组（待滤波）
  - data_len: 数组长度
  - limit: 限幅阈值（两次采样允许的最大偏差值）
输出：
  - filtered_data: 滤波后的结果数组（需提前分配内存）
功能：
  1. 以数组第一个元素作为初始有效值
  2. 遍历数组，逐次判断当前值与上一有效值的偏差
  3. 偏差超过阈值则沿用上次有效值，否则使用当前值
*/
void amplitude_limit_filter(const uint16_t *raw_data, uint16_t *filtered_data, uint16_t data_len, uint16_t limit)
{
       // 入参合法性检查（避免空指针/无效长度）
       if (raw_data == NULL || filtered_data == NULL || data_len == 0)
       {
              // 可根据需求添加错误提示，比如：
              // print_host("参数错误：数组为空或长度为0");
              return;
       }

       // 步骤1：初始化 - 第一个元素作为初始有效值
       uint16_t last_valid_value = raw_data[0];
       filtered_data[0] = last_valid_value; // 第一个值直接保留

       // 步骤2：遍历数组，逐元素进行限幅滤波
       for (uint16_t i = 1; i < data_len; i++)
       {
              uint16_t current_value = raw_data[i]; // 当前待滤波值

              // 核心逻辑：判断偏差是否超过阈值
              if (abs((int16_t)current_value - (int16_t)last_valid_value) > (int16_t)limit)
              {
                     // 偏差超阈值，舍弃当前值，沿用上次有效值
                     filtered_data[i] = last_valid_value;
              }
              else
              {
                     // 偏差在阈值内，当前值有效，更新有效值
                     filtered_data[i] = current_value;
                     last_valid_value = current_value;
              }

              // 如果你需要保留原代码中的打印逻辑，可在此处添加
              // print_FireWater(2, raw_data[i] * 1.0, filtered_data[i] * 1.0);
       }
}

/*
通用版中位值滤波函数
方法：
  1. 从原始数据数组中连续取N个采样值（N必须为奇数）
  2. 将这N个值按大小排序
  3. 取排序后中间位置的值作为本次滤波结果
输入：
  - raw_data: 原始采样数据数组（待滤波）
  - start_idx: 从原始数组的哪个索引开始取N个值（避免越界）
  - sample_num: 连续采样次数N（必须为奇数，如3、5、7）
输出：
  - 返回值：滤波后的中位值；若参数非法，返回0
优点：克服偶然因素，对温度、液位等慢变参数滤波效果好
缺点：不适用于流量、速度等快速变化的参数
*/
uint16_t median_filter(const uint16_t *raw_data, uint16_t data_len, uint16_t start_idx, uint16_t sample_num)
{
       // 1. 入参合法性检查（核心：避免数组越界、N为偶数/无效值）
       // 检查原始数组是否为空、采样次数是否为奇数且大于0、起始索引+N不超过数组长度
       if (raw_data == NULL || sample_num == 0 || (sample_num % 2) == 0 ||
           start_idx + sample_num > data_len || start_idx >= data_len)
       {
              // 参数非法时返回0（也可根据需求改为返回错误码/打印提示）
              return 0;
       }

       // 2. 临时缓冲区：存储待排序的N个采样值
       uint16_t value_buf[sample_num];
       // 从原始数组中复制N个值到临时缓冲区
       memcpy(value_buf, &raw_data[start_idx], sample_num * sizeof(uint16_t));
       // 3. 冒泡排序（从小到大）：对N个采样值排序
       uint16_t i, j, temp;
       for (j = 0; j < sample_num - 1; j++) // 外层循环：控制排序轮数
       {
              for (i = 0; i < sample_num - 1 - j; i++) // 内层循环：每轮比较次数递减
              {
                     if (value_buf[i] > value_buf[i + 1])
                     {
                            // 交换两个值的位置
                            temp = value_buf[i];
                            value_buf[i] = value_buf[i + 1];
                            value_buf[i + 1] = temp;
                     }
              }
       }
       // 4. 取中间值返回（N为奇数，中间索引为 (sample_num-1)/2）
       return value_buf[(sample_num - 1) / 2];
}

/**
 * @brief  通用版算术平均滤波函数
 * @param  raw_data: 原始采样数据数组（const修饰，避免误修改）
 * @param  data_len: 原始数组的总长度
 * @param  start_idx: 滤波起始索引（从该位置开始取N个值）
 * @param  sample_num: 平均采样次数N（如流量取12，压力取4）
 * @retval 滤波后的算术平均值；参数非法时返回0
 * @note   1. N越大平滑度越高，但灵敏度越低；
 *         2. 使用uint32_t累加和，避免多个uint16_t值相加溢出；
 *         3. 需保证start_idx + sample_num <= data_len，否则返回0
 */
uint16_t arithmetic_average_filter(const uint16_t *raw_data, uint16_t data_len,
                                   uint16_t start_idx, uint16_t sample_num)
{
       // 入参合法性检查：空指针/无效长度/采样数为0/索引越界
       if (raw_data == NULL || data_len == 0 || sample_num == 0 ||
           start_idx >= data_len || start_idx + sample_num > data_len)
       {
              return 0;
       }

       uint32_t sum = 0; // 32位累加和，防止溢出
       // 累加N个采样值
       for (uint16_t i = 0; i < sample_num; i++)
       {
              sum += raw_data[start_idx + i];
       }

       // 返回算术平均值（整数除法，向下取整）
       return (uint16_t)(sum / sample_num);
}

/**
 * @brief  通用版递推平均（滑动平均）滤波函数
 * @param  queue_buf: 滑动队列缓冲区（需提前初始化，长度为queue_len）
 * @param  queue_len: 队列长度N（固定值，如流量取12，温度取1-4）
 * @param  new_value: 新采样值（放入队列尾部）
 * @retval 滤波后的滑动平均值；参数非法时返回0
 * @note   1. 队列遵循先进先出原则，新值入队尾，旧值从队首出队；
 *         2. 需保持queue_buf的生命周期（全局/静态/堆内存），每次调用会更新队列；
 *         3. 首次使用前建议将queue_buf初始化为0或初始采样值
 */
uint16_t recursive_average_filter(uint16_t *queue_buf, uint16_t queue_len, uint16_t new_value)
{
       // 入参合法性检查：空指针/无效队列长度
       if (queue_buf == NULL || queue_len == 0)
       {
              return 0;
       }

       uint32_t sum = 0;
       // 1. 队列左移：扔掉队首元素，所有数据前移一位
       for (uint16_t i = 0; i < queue_len - 1; i++)
       {
              queue_buf[i] = queue_buf[i + 1];
       }
       // 2. 新值放入队尾
       queue_buf[queue_len - 1] = new_value;

       // 3. 计算队列内所有值的累加和
       for (uint16_t i = 0; i < queue_len; i++)
       {
              sum += queue_buf[i];
       }

       // 4. 返回滑动平均值
       return (uint16_t)(sum / queue_len);
}

/**
 * @brief  通用版中位值平均滤波函数（防脉冲干扰平均）
 * @param  raw_data: 原始采样数据数组
 * @param  data_len: 原始数组总长度
 * @param  start_idx: 起始索引（从该位置取N个值）
 * @param  sample_num: 采样次数N（建议3-14，需≥3，否则无法去最大最小值）
 * @retval 去掉最大最小值后的平均值；参数非法返回0
 * @note   融合中位值+算术平均，抗脉冲干扰能力强
 */
uint16_t median_average_filter(const uint16_t *raw_data, uint16_t data_len,
                               uint16_t start_idx, uint16_t sample_num)
{
       // 入参检查：N≥3、索引不越界
       if (raw_data == NULL || data_len == 0 || sample_num < 3 ||
           start_idx >= data_len || start_idx + sample_num > data_len)
       {
              return 0;
       }

       // 1. 复制N个采样值到临时缓冲区
       uint16_t filter_buf[sample_num];
       memcpy(filter_buf, &raw_data[start_idx], sample_num * sizeof(uint16_t));

       // 2. 冒泡排序（从小到大）
       uint16_t i, j, temp;
       for (j = 0; j < sample_num - 1; j++)
       {
              for (i = 0; i < sample_num - 1 - j; i++)
              {
                     if (filter_buf[i] > filter_buf[i + 1])
                     {
                            temp = filter_buf[i];
                            filter_buf[i] = filter_buf[i + 1];
                            filter_buf[i + 1] = temp;
                     }
              }
       }

       // 3. 去掉最大值和最小值，累加中间N-2个值
       uint32_t sum = 0;
       for (i = 1; i < sample_num - 1; i++)
       {
              sum += filter_buf[i];
       }

       // 4. 返回平均值
       return (uint16_t)(sum / (sample_num - 2));
}

/**
 * @brief  通用版限幅平均滤波函数
 * @param  queue_buf: 滑动队列缓冲区（长度为queue_len，需初始化）
 * @param  queue_len: 队列长度N
 * @param  new_value: 新采样值
 * @param  limit: 限幅阈值A（两次采样允许的最大偏差）
 * @retval 限幅后滑动平均的结果；参数非法返回0
 * @note   融合限幅+递推平均，先限幅再平均，抗脉冲干扰
 */
uint16_t amplitude_limit_average_filter(uint16_t *queue_buf, uint16_t queue_len,
                                        uint16_t new_value, uint16_t limit)
{
       if (queue_buf == NULL || queue_len < 2) // 队列至少长度2才能限幅
       {
              return 0;
       }

       uint16_t limited_value = new_value;
       // 限幅处理：新值与队列最后一个值（上一个值）比较
       if (abs((int16_t)limited_value - (int16_t)queue_buf[queue_len - 1]) > (int16_t)limit)
       {
              limited_value = queue_buf[queue_len - 1]; // 超阈值则用上次值替代
       }

       // 调用递推平均滤波，处理限幅后的值
       return recursive_average_filter(queue_buf, queue_len, limited_value);
}

/**
 * @brief  通用版一阶滞后滤波函数
 * @param  new_value: 本次采样值
 * @param  last_result: 上次滤波结果
 * @param  alpha: 滞后系数（0≤alpha≤1，alpha越大平滑度越高，滞后越明显）
 * @retval 本次滤波结果；参数非法返回last_result
 * @note   公式：本次结果 = alpha*上次结果 + (1-alpha)*本次采样值
 */
uint16_t first_order_lag_filter(uint16_t new_value, uint16_t last_result, float alpha)
{
       // 入参检查：alpha需在0~1之间
       if (alpha < 0.0f || alpha > 1.0f)
       {
              return last_result;
       }

       // 一阶滞后核心公式（浮点运算后转整数）
       float result = alpha * (float)last_result + (1.0f - alpha) * (float)new_value;
       return (uint16_t)roundf(result); // 四舍五入，避免精度丢失
}

/**
 * @brief  通用版加权递推平均滤波函数
 * @param  queue_buf: 滑动队列缓冲区（长度=queue_len，需初始化）
 * @param  queue_len: 队列长度N
 * @param  coe: 加权系数数组（长度=queue_len，越新数据权值建议越大）
 * @param  sum_coe: 加权系数总和（避免重复计算）
 * @param  new_value: 新采样值（入队尾）
 * @retval 加权平均结果；参数非法返回0
 * @note   权值越大的系数对应越新的数据，灵敏度越高
 */
uint16_t weighted_recursive_average_filter(uint16_t *queue_buf, uint16_t queue_len,
                                           const uint16_t *coe, uint32_t sum_coe,
                                           uint16_t new_value)
{
       // 入参检查：空指针、无效长度、系数和为0
       if (queue_buf == NULL || coe == NULL || queue_len == 0 || sum_coe == 0)
       {
              return 0;
       }

       // 1. 队列左移，扔掉队首
       for (uint16_t i = 0; i < queue_len - 1; i++)
       {
              queue_buf[i] = queue_buf[i + 1];
       }
       // 2. 新值入队尾
       queue_buf[queue_len - 1] = new_value;

       // 3. 加权求和
       uint32_t filter_sum = 0;
       for (uint16_t i = 0; i < queue_len; i++)
       {
              filter_sum += (uint32_t)queue_buf[i] * coe[i];
       }

       // 4. 加权平均（整数除法）
       return (uint16_t)(filter_sum / sum_coe);
}

/**
 * @brief  通用版消抖滤波函数
 * @param  new_value: 本次采样值
 * @param  last_valid: 上次有效值（需传指针，保持状态）
 * @param  counter: 滤波计数器（需传指针，保持状态）
 * @param  max_count: 消抖上限N（溢出则更新有效值）
 * @retval 当前有效值；参数非法返回*last_valid
 * @note   采样值与有效值一致则清零计数器，否则累加，溢出则更新
 */
uint16_t debounce_filter(uint16_t new_value, uint16_t *last_valid,
                         uint16_t *counter, uint16_t max_count)
{
       // 入参检查
       if (last_valid == NULL || counter == NULL)
       {
              return (last_valid != NULL) ? *last_valid : 0;
       }

       if (new_value != *last_valid)
       {
              (*counter)++;
              // 计数器溢出，更新有效值并清零
              if (*counter > max_count)
              {
                     *counter = 0;
                     *last_valid = new_value;
              }
       }
       else
       {
              // 采样值一致，清零计数器
              *counter = 0;
       }

       return *last_valid;
}

/**
 * @brief  通用版限幅消抖滤波函数
 * @param  new_value: 本次采样值
 * @param  last_valid: 上次有效值（需传指针，保持状态）
 * @param  counter: 滤波计数器（需传指针，保持状态）
 * @param  limit: 限幅阈值A
 * @param  max_count: 消抖上限N
 * @retval 当前有效值；参数非法返回*last_valid
 * @note   先限幅再消抖，避免干扰值进入消抖逻辑
 */
uint16_t amplitude_limit_debounce_filter(uint16_t new_value, uint16_t *last_valid,
                                         uint16_t *counter, uint16_t limit,
                                         uint16_t max_count)
{
       // 入参检查
       if (last_valid == NULL || counter == NULL)
       {
              return (last_valid != NULL) ? *last_valid : 0;
       }

       // 1. 先限幅处理
       uint16_t limited_value = new_value;
       if (abs((int16_t)limited_value - (int16_t)*last_valid) > (int16_t)limit)
       {
              limited_value = *last_valid;
       }
       // 2. 再消抖处理（复用消抖逻辑）
       return debounce_filter(limited_value, last_valid, counter, max_count);
}
