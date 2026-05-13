#ifndef ANALOG_MANAGER_H
#define ANALOG_MANAGER_H

#include <stdint.h>

/* ── 模拟量统一数据结构 ─────────────────────────── */
typedef struct {
    float    current;          /* ACS712 电流 (A)         */
    int32_t  encoder_count;    /* 编码器原始计数值         */
    uint32_t encoder_rpm;      /* 编码器转速 (RPM)        */
    uint16_t adc_raw[3];       /* ADC CH3/CH4/CH5 原始值  */
    uint32_t timestamp_ms;
} analog_data_t;

/* ── 状态位 ──────────────────────────────────────── */
#define ANALOG_OK_ADC       (1u << 0)
#define ANALOG_OK_ENCODER   (1u << 1)
#define ANALOG_OK_CURRENT   (1u << 2)

/* ── API ────────────────────────────────────────── */

/**
 * 初始化 ADC DMA 采样 + ACS712 电流传感器 + TIM3 编码器
 * @return 位掩码，表示各子系统初始化是否成功
 */
uint8_t AnalogManager_Init(void);

/**
 * 启动 ADC DMA 采样 + 编码器计数
 * @return 0 成功，非 0 失败
 */
int AnalogManager_Start(void);

/**
 * 停止 ADC DMA 采样 + 编码器计数
 */
void AnalogManager_Stop(void);

/**
 * 读取全部模拟量数据（电流、编码器、ADC 原始值）
 * @param data 输出数据
 * @return 位掩码，表示各子系统本次读取是否成功
 */
uint8_t AnalogManager_ReadAll(analog_data_t *data);

/**
 * 设置 ACS712 零电流偏移电压
 * @param zero_v 零电流时 ADC 输入电压 (V)，默认 1.65
 */
void AnalogManager_SetCurrentOffset(float zero_v);

/**
 * 设置编码器每转脉冲数
 * @param ppr 脉冲数/转，默认 1024
 */
void AnalogManager_SetEncoderPPR(uint32_t ppr);

#endif /* ANALOG_MANAGER_H */
