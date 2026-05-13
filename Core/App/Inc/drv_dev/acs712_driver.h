#ifndef ACS712_DRIVER_H
#define ACS712_DRIVER_H

#include <stdint.h>

typedef struct {
    float zero_offset;     /* 零电流时 ADC 电压 (V)，如 1.65V 或 1.67V */
    float sensitivity;     /* 灵敏度 (V/A)，典型值 0.185 (5V供电) 或 0.122 (3.3V) */
    float adc_ref;         /* ADC 参考电压 (V)，通常 3.3 */
    uint16_t adc_raw;      /* 上一次 ADC 原始值（调试用） */
} acs712_dev_t;

/**
 * 初始化 ACS712 设备结构体
 * @param dev        设备结构体指针
 * @param zero_volts 零电流时的 ADC 电压 (V)
 * @param sens       灵敏度 (V/A)，ACS712-05B: 0.185 @5V
 * @param ref_volts  ADC 参考电压 (V)
 */
void ACS712_Init(acs712_dev_t *dev, float zero_volts, float sens, float ref_volts);

/**
 * 读取电流值（阻塞，从 ADC DMA 缓冲区取最新值）
 * @param dev    设备结构体指针
 * @param adc_buf ADC DMA 循环缓冲区基址
 * @param channel 缓冲区中的通道索引 (0/1/2 对应 CH3/CH4/CH5)
 * @return 电流值 (A)
 */
float ACS712_ReadCurrent(const acs712_dev_t *dev, const uint32_t *adc_buf, int channel);

#endif
