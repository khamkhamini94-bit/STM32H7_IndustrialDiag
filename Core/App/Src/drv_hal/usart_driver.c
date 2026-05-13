/* usart_driver.c - USART1 DMA driver and USART3 IRQ driver */
#include "drv_hal/usart_driver.h"
#include "drv_hal/dma_driver.h"
#include "stm32h7xx_hal.h"

/* extern handles declared in generated usart.c */
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

/* USART1 DMA state */
static volatile uint8_t *s_usart1_rx_buf = NULL;
static volatile uint32_t s_usart1_rx_len = 0;
static volatile usart_rx_half_cb_t s_usart1_rx_half_cb = NULL;
static volatile usart_rx_full_cb_t s_usart1_rx_full_cb = NULL;
static volatile usart_tx_cb_t s_usart1_tx_cb = NULL;
static volatile usart_err_cb_t s_usart1_err_cb = NULL;

/* USART3 IRQ state */
static volatile uint8_t *s_usart3_rx_buf = NULL;
static volatile uint32_t s_usart3_rx_len = 0;
static volatile usart_rx_full_cb_t s_usart3_rx_cb = NULL;
static volatile usart_tx_cb_t s_usart3_tx_cb = NULL;
static volatile usart_err_cb_t s_usart3_err_cb = NULL;

/* --- USART1 (DMA) --- */
int USART1_SendBlocking(uint8_t *buf, uint32_t len, uint32_t timeout_ms)
{
    if (buf == NULL || len == 0) return -1;
    return (HAL_UART_Transmit(&huart1, buf, len, timeout_ms) != HAL_OK) ? -1 : 0;
}

int USART1_SendDMA(uint8_t *buf, uint32_t len)
{
    if (buf == NULL || len == 0) return -1;
    /* Check busy state */
    if (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY) return -1;
    DMA_CleanBuffer(buf, len);
    if (HAL_UART_Transmit_DMA(&huart1, buf, len) != HAL_OK) return -1;
    return 0;
}

int USART1_StartRxDMA(uint8_t *buf, uint32_t len)
{
    if (buf == NULL || len == 0) return -1;
    s_usart1_rx_buf = buf;
    s_usart1_rx_len = len;
    /* Ensure CPU doesn't read stale cache lines while DMA writes */
    DMA_InvalidateBuffer(buf, len);
    if (HAL_UART_Receive_DMA(&huart1, (uint8_t *)buf, len) != HAL_OK) return -1;
    return 0;
}

int USART1_StopRxDMA(void)
{   
    if (HAL_UART_AbortReceive(&huart1) != HAL_OK) return -1;
    s_usart1_rx_buf = NULL;
    s_usart1_rx_len = 0;
    return 0;
}

uint32_t USART1_GetRxWriteIndex(void)
{
    if (s_usart1_rx_buf == NULL || s_usart1_rx_len == 0) return 0;
    /* DMA counter gives remaining transfers */
    uint32_t rem = __HAL_DMA_GET_COUNTER(huart1.hdmarx);
    return (uint32_t)(s_usart1_rx_len - rem) % s_usart1_rx_len;
}

void USART1_RegisterRxCallbacks(usart_rx_half_cb_t half_cb, usart_rx_full_cb_t full_cb)
{
    __disable_irq();
    s_usart1_rx_half_cb = half_cb;
    s_usart1_rx_full_cb = full_cb;
    __enable_irq();
}

void USART1_RegisterTxCallback(usart_tx_cb_t tx_cb)
{
    __disable_irq(); s_usart1_tx_cb = tx_cb; __enable_irq();
}

void USART1_RegisterErrorCallback(usart_err_cb_t err_cb)
{
    __disable_irq(); s_usart1_err_cb = err_cb; __enable_irq();
}

/* --- USART3 (IRQ) --- */
int USART3_SendBlocking(uint8_t *buf, uint32_t len, uint32_t timeout_ms)
{
    if (buf == NULL || len == 0) return -1;
    return (HAL_UART_Transmit(&huart3, buf, len, timeout_ms) != HAL_OK) ? -1 : 0;
}

int USART3_SendIT(uint8_t *buf, uint32_t len)
{
    if (buf == NULL || len == 0) return -1;
    if (HAL_UART_Transmit_IT(&huart3, buf, len) != HAL_OK) return -1;
    return 0;
}

int USART3_StartRxIT(uint8_t *buf, uint32_t len)
{
    if (buf == NULL || len == 0) return -1;
    s_usart3_rx_buf = buf;
    s_usart3_rx_len = len;
    if (HAL_UART_Receive_IT(&huart3, buf, len) != HAL_OK) return -1;
    return 0;
}

int USART3_StopRxIT(void)
{
    if (HAL_UART_AbortReceive_IT(&huart3) != HAL_OK) return -1;
    s_usart3_rx_buf = NULL;
    s_usart3_rx_len = 0;
    return 0;
}

void USART3_RegisterRxCallback(usart_rx_full_cb_t cb)
{
    __disable_irq(); s_usart3_rx_cb = cb; __enable_irq();
}

void USART3_RegisterTxCallback(usart_tx_cb_t cb)
{
    __disable_irq(); s_usart3_tx_cb = cb; __enable_irq();
}

void USART3_RegisterErrorCallback(usart_err_cb_t cb)
{
    __disable_irq(); s_usart3_err_cb = cb; __enable_irq();
}

/* --- HAL Callbacks routing --- */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        if (s_usart1_tx_cb) s_usart1_tx_cb();
    }
    else if (huart == &huart3)
    {
        if (s_usart3_tx_cb) s_usart3_tx_cb();
    }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        /* Invalidate first half so CPU sees DMA-written data */
        if (s_usart1_rx_buf && s_usart1_rx_len)
        {
            DMA_InvalidateBuffer((void *)s_usart1_rx_buf, s_usart1_rx_len/2);
        }
        if (s_usart1_rx_half_cb) s_usart1_rx_half_cb();
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        /* Invalidate second half */
        if (s_usart1_rx_buf && s_usart1_rx_len)
        {
            uint8_t *p = (uint8_t *)s_usart1_rx_buf + (s_usart1_rx_len/2);
            DMA_InvalidateBuffer((void *)p, s_usart1_rx_len - s_usart1_rx_len/2);
        }
        if (s_usart1_rx_full_cb) s_usart1_rx_full_cb();
    }
    else if (huart == &huart3)
    {
        if (s_usart3_rx_cb) s_usart3_rx_cb();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        if (s_usart1_err_cb) s_usart1_err_cb(huart);
    }
    else if (huart == &huart3)
    {
        if (s_usart3_err_cb) s_usart3_err_cb(huart);
    }
}

/* --- Initialization and printf redirection --- */
void USART_Drivers_Init(void)
{
    /* Placeholder: user should call MX_USART1_UART_Init()/MX_USART3_UART_Init() before this if using CubeMX init order.
       _write() uses huart1 for printf redirection. */
}

int _write(int file, char *ptr, int len)
{
    (void)file;
    if (huart1.Instance != NULL)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, 1000);
    }
    return len;
}
