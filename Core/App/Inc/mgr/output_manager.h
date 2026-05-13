#ifndef OUTPUT_MANAGER_H
#define OUTPUT_MANAGER_H

#include <stdint.h>

/* ── 状态位 ──────────────────────────────────────── */
#define OUTPUT_OK_PWM      (1u << 0)
#define OUTPUT_OK_LED      (1u << 1)
#define OUTPUT_OK_BUZZER   (1u << 2)

/* ── LED 状态 ────────────────────────────────────── */
typedef enum {
    LED_OFF = 0,
    LED_ON  = 1,
} led_state_t;

/* ── API ────────────────────────────────────────── */

/** 初始化：检查 PWM 定时器、LED/Buzzer GPIO 是否就绪 */
uint8_t OutputManager_Init(void);

/* PWM（TIM1 CH1，0-100%） */
void OutputManager_SetPWM(uint8_t duty);
void OutputManager_EmergencyStop(void);
void OutputManager_PWMResume(void);

/* LED */
void OutputManager_SetGreen(led_state_t s);
void OutputManager_SetYellow(led_state_t s);
void OutputManager_SetRed(led_state_t s);

/* Buzzer */
void OutputManager_SetBuzzer(uint8_t on);

#endif /* OUTPUT_MANAGER_H */
