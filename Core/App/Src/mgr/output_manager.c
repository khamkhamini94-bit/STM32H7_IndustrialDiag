#include "mgr/output_manager.h"
#include "drv_hal/tim_driver.h"
#include "gpio.h"
#include "stm32h7xx_hal.h"

/* ── PWM 状态记录 ────────────────────────────────── */
static uint8_t s_pwm_estopped = 0;

/* ── 初始化 ──────────────────────────────────────── */
uint8_t OutputManager_Init(void)
{
    uint8_t ok_mask = 0;

    /* PWM 定时器（TIM1 CH1 已在 MX_TIM1_Init 中初始化）*/
    if (TIM1_PWM_Start() == 0) {
        ok_mask |= OUTPUT_OK_PWM;
    }

    /* LED / Buzzer GPIO 假设 CubeMX 已配置 */
    ok_mask |= OUTPUT_OK_LED | OUTPUT_OK_BUZZER;

    /* 初始状态：绿灯亮，其余灭 */
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port,  LED_GREEN_Pin,  GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_RED_GPIO_Port,    LED_RED_Pin,    GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BUZZER_GPIO_Port,     BUZZER_Pin,     GPIO_PIN_RESET);

    return ok_mask;
}

/* ── PWM ─────────────────────────────────────────── */
void OutputManager_SetPWM(uint8_t duty)
{
    if (s_pwm_estopped) return;
    TIM1_PWM_SetDuty(duty);
}

void OutputManager_EmergencyStop(void)
{
    s_pwm_estopped = 1;
    TIM1_PWM_EmergencyStop();
}

void OutputManager_PWMResume(void)
{
    s_pwm_estopped = 0;
    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1_PWM_Start();
}

/* ── LED ─────────────────────────────────────────── */
void OutputManager_SetGreen(led_state_t s)
{
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin,
        (s == LED_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void OutputManager_SetYellow(led_state_t s)
{
    HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin,
        (s == LED_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void OutputManager_SetRed(led_state_t s)
{
    HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin,
        (s == LED_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* ── Buzzer ──────────────────────────────────────── */
void OutputManager_SetBuzzer(uint8_t on)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin,
        on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
