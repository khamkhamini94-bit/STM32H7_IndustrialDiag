#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "adc.h"

typedef void (*adc_conv_cb_t)(void);

/**
 * 初始化 ADC 驱动（调用 CubeMX 生成的 MX_ADC1_Init）
 */
void ADC_Driver_Init(void);

/**
 * 查询驱动状态
 * @return 0 表示 OK，负值表示错误或忙
 */
int32_t ADC_Driver_GetStatus(void);
/**
 * 使用 DMA 启动 ADC 采样
 * @param buffer 指向接收缓冲区（必须为 uint32_t 对齐）
 * @param length 要采集的样点数量
 * @return 0 成功，非 0 失败
 */
int ADC_Driver_StartDMA(uint32_t *buffer, uint32_t length);

/**
 * 停止 ADC DMA 采样
 */
void ADC_Driver_StopDMA(void);

/**
 * 注册转换完成回调（在中断上下文被调用）
 */
void ADC_Driver_RegisterConvCpltCallback(adc_conv_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // ADC_DRIVER_H
