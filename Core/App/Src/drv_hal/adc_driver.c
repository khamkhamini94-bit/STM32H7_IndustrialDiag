/* adc_driver.c - simple ADC wrapper for DMA-based sampling */
#include "drv_hal/adc_driver.h"
#include "drv_hal/dma_driver.h"
#include "main.h"
#include "stm32h7xx_hal.h"

static volatile uint32_t *s_buffer = NULL;
static volatile uint32_t s_length = 0;
static volatile adc_conv_cb_t s_conv_cb = NULL;

typedef enum {
    ADC_DRV_OK    = 0,
    ADC_DRV_ERROR = -1,
    ADC_DRV_BUSY  = -2
} ADC_Driver_Status_t;

int32_t ADC_Driver_GetStatus(void)
{
    uint32_t st = HAL_ADC_GetState(&hadc1);
    if (st == HAL_ADC_STATE_ERROR)
        return ADC_DRV_ERROR;
    if (st == HAL_ADC_STATE_REG_BUSY ||
        st == HAL_ADC_STATE_INJ_BUSY ||
        st == HAL_ADC_STATE_BUSY_INTERNAL)
        return ADC_DRV_BUSY;
    return ADC_DRV_OK;
}

void ADC_Driver_Init(void)
{
    /* MX_ADC1_Init() 已在 main.c 中调用，此处仅执行校准 */
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }
}

int ADC_Driver_StartDMA(uint32_t *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) return ADC_DRV_ERROR;
    if (ADC_Driver_GetStatus() == ADC_DRV_BUSY) return ADC_DRV_BUSY;

    s_buffer = buffer;
    s_length = length;

    DMA_InvalidateBuffer((void *)s_buffer, s_length * sizeof(uint32_t));

    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)s_buffer, (uint32_t)s_length) != HAL_OK)
        return ADC_DRV_ERROR;

    return ADC_DRV_OK;
}

void ADC_Driver_StopDMA(void)
{
    HAL_ADC_Stop_DMA(&hadc1);
    s_buffer = NULL;
    s_length = 0;
}

void ADC_Driver_RegisterConvCpltCallback(adc_conv_cb_t cb)
{   
    __disable_irq();

    s_conv_cb = cb;

    __enable_irq(); 
}

/* HAL 回调：转换完成（当使用 DMA 并配置为循环时，此回调在每个缓冲区完成时触发） */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        /* 在 DMA 完成后通过 dma_driver 接口失效对应缓存，保证 CPU 读取到 DMA 写入的数据 */
        DMA_InvalidateBuffer((void *)s_buffer, s_length * sizeof(uint32_t));

        if (s_conv_cb)
        {
            s_conv_cb();
        }
    }
}

/* 可选：半转换回调 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    /* 留空或用于流式处理 */
}

/* 可选：DMA 错误回调 */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        // 停止 DMA 防止继续出错
        ADC_Driver_StopDMA();
        // 通知上层（通过已注册的回调或全局标志）
        // 这里可以设置一个错误标志
        // 让 sensor_task 检测到后触发状态机 FAULT
    }
}