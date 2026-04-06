/**
 ****************************************************************************************************
 * @file        adc1.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-01
 * @brief       ADC驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 ESP32-P4 开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 * 
 ****************************************************************************************************
 */

#include "adc.h"


adc_oneshot_unit_handle_t adcx_handle = NULL;        /* ADC句柄 */
adc_cali_handle_t adcx_cali_chanx_handle = NULL;    /* ADC校准句柄 */

/**
 * @brief       ADC校准初始化
 * @param       unit        : ADC单元
 * @param       channel     : ADC单元通道
 * @param       atten       : ADC衰减数值
 * @param       out_handle  : ADC校准句柄
 * @retval      true:校准成功;false:校准失败
 */
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    if (!calibrated)
    {
        ESP_LOGI("adc", "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);

        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }

    *out_handle = handle;

    if (ret == ESP_OK)
    {
        ESP_LOGI("adc", "Calibration Success");
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW("adc", "eFuse not burnt, skip software calibration");
    }
    else
    {
        ESP_LOGE("adc", "Invalid arg or no memory");
    }

    return calibrated;
}

/**
 * @brief       初始化ADC
 * @param       无
 * @retval      无
 */
void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t adc_config = {  /* 初始化配置结构体 */
        .unit_id  = ADC_UNIT_X,                 /* ADC单元:ADC1/ADC2 */
        .ulp_mode = ADC_ULP_MODE_DISABLE,       /* 不支持ADC在ULP模式下工作 */
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_config, &adcx_handle));                /* ADC初始化(单次转换模式) */

    /* 配置 ADC */
    adc_oneshot_chan_cfg_t config = {
        .atten    = ADC_ATTEN,                  /* ADC衰减 */
        .bitwidth = ADC_BITWIDTH,               /* ADC分辨率 */
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adcx_handle, ADC_CHAN, &config));     /* 配置ADC通道 */

    /* ADC校准 */
    adc_calibration_init(ADC_UNIT_X, ADC_CHAN, ADC_ATTEN, &adcx_cali_chanx_handle);
}

#define LOST_VAL    1

/**
 * @brief       获取ADC转换的电压数值且进行多次采样后排序去除最高和最低值再做均值滤波后的结果
 * @note        ESP32P4 ADC对噪声敏感,可能导致ADC读数出现较大偏差
 * @note        软件上:可通过多次采样进一步降低噪声影响;硬件上:可加旁路电容连在在ADC使用引脚上
 * @param       times   : 获取次数
 * @retval      通道ch的times次转换电压平均值
 */
int adc_get_result_voltage(uint32_t times)
{
    uint32_t sum = 0;
    int voltage = 0;
    uint16_t temp_val = 0;

    /* 申请存放ADC原始数据buffer */
    int *rawdata = heap_caps_malloc(times * sizeof(int), MALLOC_CAP_INTERNAL);     
    if (NULL == rawdata)
    {
        ESP_LOGE("adc", "Memory for adc is not enough");
    }

    for (uint32_t t = 0; t < times; t++)                        /* 多次ADC采样 */
    {
        adc_oneshot_read(adcx_handle, ADC_CHAN, &rawdata[t]);    /* 读取原始数据 */
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    for (uint16_t i = 0; i < times - 1; i++)                    /* 对数据进行排序 */
    {
        for (uint16_t j = i + 1; j < times; j++)
        {
            if (rawdata[i] > rawdata[j])                        /* 升序排列 */
            {
                temp_val   = rawdata[i];
                rawdata[i] = rawdata[j];
                rawdata[j] = temp_val;
            }
        }
    }

    for (uint32_t i = LOST_VAL; i < times - LOST_VAL; i++)      /* 去掉两端的丢弃值 */
    {
        sum += rawdata[i];                                      /* 累加去掉丢弃值以后的数据 */
    }

    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adcx_cali_chanx_handle, (sum / (times - 2 * LOST_VAL)), &voltage));

    return voltage;                                             /* 返回平均值 */
}

/**
 * @brief       关闭ADCx
 * @param       无
 * @retval      无
 */
void adc_deinit(void)
{
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adcx_handle));
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(adcx_cali_chanx_handle));
}
