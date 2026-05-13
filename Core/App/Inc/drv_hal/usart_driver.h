/* usart_driver.h - Lightweight drivers for USART1 (DMA) and USART3 (IRQ) */
#ifndef USART_DRIVER_H
#define USART_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "usart.h"

typedef void (*usart_rx_half_cb_t)(void);   /* called on half-buffer received for USART1 */
typedef void (*usart_rx_full_cb_t)(void);   /* called on full-buffer received for USART1 */
typedef void (*usart_tx_cb_t)(void);        /* called on TX complete */
typedef void (*usart_err_cb_t)(UART_HandleTypeDef *huart); /* error callback */

/* USART1 (DMA) API */
int USART1_SendBlocking(uint8_t *buf, uint32_t len, uint32_t timeout_ms);
int USART1_SendDMA(uint8_t *buf, uint32_t len);

int USART1_StartRxDMA(uint8_t *buf, uint32_t len);
int USART1_StopRxDMA(void);
/** 返回 DMA 写入的当前索引（0..len-1） */
uint32_t USART1_GetRxWriteIndex(void);

void USART1_RegisterRxCallbacks(usart_rx_half_cb_t half_cb, usart_rx_full_cb_t full_cb);
void USART1_RegisterTxCallback(usart_tx_cb_t tx_cb);
void USART1_RegisterErrorCallback(usart_err_cb_t err_cb);

/* 初始化驱动（可在 main 中调用，完成 printf 重定向等） */
void USART_Drivers_Init(void);

/* USART3 (IRQ) API - simple interrupt-driven RX/TX */
int USART3_SendBlocking(uint8_t *buf, uint32_t len, uint32_t timeout_ms);
int USART3_SendIT(uint8_t *buf, uint32_t len);

int USART3_StartRxIT(uint8_t *buf, uint32_t len);
int USART3_StopRxIT(void);

void USART3_RegisterRxCallback(usart_rx_full_cb_t cb);
void USART3_RegisterTxCallback(usart_tx_cb_t cb);
void USART3_RegisterErrorCallback(usart_err_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // USART_DRIVER_H
