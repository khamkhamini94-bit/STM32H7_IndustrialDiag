/* tim_driver.h - Lightweight TIM driver wrapper for TIM1 (PWM) and TIM3 (Encoder) */
#ifndef TIM_DRIVER_H
#define TIM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "tim.h"

typedef void (*tim_period_cb_t)(void);

/**
 * 初始化 TIM 驱动（可选，安全的幂等初始化）
 *
 * 说明：应在 `MX_TIMx_Init()` 之后调用一次以清理内部状态。
 */
void TIM_Manager_Init(void);

/* TIM1 PWM 接口（通道 1） */
int TIM1_PWM_Start(void);
int TIM1_PWM_Stop(void);
int TIM1_PWM_SetPulse(uint32_t pulse);

/**
 * 设置 PWM 占空比（0-100），返回 0 成功，-1 失败
 */
int TIM1_PWM_SetDuty(uint8_t duty_percent);

/**
 * 紧急停止 PWM 输出（切断 MOE），会直接操作 TIM1 寄存器。
 * 注意：调用后需显式恢复 MOE 与 CCR 值以恢复输出。
 */
void TIM1_PWM_EmergencyStop(void);

/* TIM3 编码器接口 */
int TIM3_Encoder_Start(void);
int TIM3_Encoder_Stop(void);
uint32_t TIM3_GetCounter(void);
void TIM3_ResetCounter(void);

/** 返回编码器方向：1 表示正向（计数增加），-1 表示反向（计数减少） */
int8_t TIM3_GetDirection(void);

/**
 * 计算转速（RPM），基于上次调用的时间差与计数差。
 * 线程安全性：函数内部会原子快照上次计数/时间并更新它们，适合在任务上下文调用。
 * pulse_per_rev: 编码器每转脉冲数（PPR）
 */
uint32_t TIM3_GetRPM(uint32_t pulse_per_rev);

/* 周期溢出回调注册（回调会在 TIM 的中断上下文被调用）
 * 回调内应尽量短小，仅设置信号量或标志，避免长时间阻塞 ISR。
 */
void TIM_RegisterPeriodElapsedCallback(TIM_HandleTypeDef *htim, tim_period_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // TIM_DRIVER_H
