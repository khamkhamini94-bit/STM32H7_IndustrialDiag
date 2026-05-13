#include "drv_hal/tim_driver.h"
#include "stm32h7xx_hal.h"

static volatile tim_period_cb_t s_tim1_cb = NULL;
static volatile tim_period_cb_t s_tim3_cb = NULL;
/* 上一次编码器计数与时间戳；并发访问受临界区保护 */
static volatile uint32_t s_last_count = 0;
static volatile uint32_t s_last_tick  = 0;

void TIM_Manager_Init(void)
{
    s_tim1_cb    = NULL;
    s_tim3_cb    = NULL;
    s_last_count = 0;
    s_last_tick  = HAL_GetTick();
}

/* PWM */
int TIM1_PWM_Start(void)
{
    return (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) ? -1 : 0;
}

int TIM1_PWM_Stop(void)
{
    return (HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1) != HAL_OK) ? -1 : 0;
}

int TIM1_PWM_SetPulse(uint32_t pulse)
{
    if (pulse > htim1.Init.Period + 1) return -1;
    __disable_irq();
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
    __enable_irq();
    return 0;
}

int TIM1_PWM_SetDuty(uint8_t duty_percent)
{
    if (duty_percent > 100) return -1;
    uint32_t pulse = (uint32_t)(((uint64_t)(htim1.Init.Period + 1) * duty_percent) / 100);
    return TIM1_PWM_SetPulse(pulse);
}

void TIM1_PWM_EmergencyStop(void)
{
    __disable_irq();
    TIM1->CCR1  = 0;
    TIM1->BDTR &= ~TIM_BDTR_MOE;
    __enable_irq();
}

/* 编码器 */
int TIM3_Encoder_Start(void)
{
    uint32_t tick = HAL_GetTick();
    __disable_irq();
    s_last_count = 0;
    s_last_tick  = tick;
    __enable_irq();
    return (HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL) != HAL_OK) ? -1 : 0;
}

int TIM3_Encoder_Stop(void)
{
    return (HAL_TIM_Encoder_Stop(&htim3, TIM_CHANNEL_ALL) != HAL_OK) ? -1 : 0;
}

uint32_t TIM3_GetCounter(void)
{
    return (uint32_t)__HAL_TIM_GET_COUNTER(&htim3);
}

void TIM3_ResetCounter(void)
{
    uint32_t tick = HAL_GetTick();
    __disable_irq();
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    s_last_count = 0;
    s_last_tick  = tick;
    __enable_irq();
}

int8_t TIM3_GetDirection(void)
{
    return (TIM3->CR1 & TIM_CR1_DIR) ? -1 : 1;
}

uint32_t TIM3_GetRPM(uint32_t pulse_per_rev)
{
    if (pulse_per_rev == 0) return 0;
    /* 快照当前计数和时间（先读时间以减少禁中断时间） */
    uint32_t now_count = __HAL_TIM_GET_COUNTER(&htim3);
    uint32_t now_tick  = HAL_GetTick();

    /* 原子地交换上次计数/时间，并获取它们的历史值用于计算 */
    uint32_t prev_count, prev_tick;
    __disable_irq();
    prev_count = s_last_count;
    prev_tick  = s_last_tick;
    s_last_count = now_count;
    s_last_tick  = now_tick;
    __enable_irq();

    uint32_t dt_ms = now_tick - prev_tick;
    if (dt_ms == 0) return 0;

    uint32_t delta;
    if (now_count >= prev_count)
    {
        delta = now_count - prev_count;
    }
    else
    {
        /* 采用定时器的周期作为回绕基数（Period+1），更鲁棒 */
        uint32_t wrap = (uint32_t)htim3.Init.Period + 1U;
        if (wrap == 0) wrap = 0x10000U;
        delta = (wrap - prev_count) + now_count;
    }

    return (uint32_t)(((uint64_t)delta * 60000UL) / ((uint64_t)pulse_per_rev * dt_ms));
}

/* 回调注册 */
void TIM_RegisterPeriodElapsedCallback(
    TIM_HandleTypeDef *htim, tim_period_cb_t cb)
{
    if (htim == NULL) return;
    if (htim->Instance == TIM1)
    {
        __disable_irq(); s_tim1_cb = cb; __enable_irq();
    }
    else if (htim->Instance == TIM3)
    {
        __disable_irq(); s_tim3_cb = cb; __enable_irq();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) return;
    if (htim->Instance == TIM1 && s_tim1_cb) s_tim1_cb();
    else if (htim->Instance == TIM3 && s_tim3_cb) s_tim3_cb();
}