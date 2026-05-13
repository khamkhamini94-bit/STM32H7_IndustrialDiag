/* dma_driver.c - DMA helper: cache management and error callback */
#include "drv_hal/dma_driver.h"
#include "stm32h7xx_hal.h"

static volatile dma_error_cb_t s_err_cb = NULL;
/* 全局错误统计（简单实现：全局计数 + 最近错误的 hdma 指针） */
static volatile uint32_t s_dma_error_count = 0;
static volatile DMA_HandleTypeDef *s_last_error_hdma = NULL;

void DMA_Manager_Init(void)
{
    s_err_cb = NULL;
    s_dma_error_count = 0;
    s_last_error_hdma = NULL;
}
void DMA_CleanBuffer(void *buf, uint32_t len)
{
    if (buf == NULL || len == 0) return;
    /* Align start down to 32-byte cache line and end up to cover full lines */
    uint32_t start = (uint32_t)buf & ~0x1FU;
    uint32_t end = (((uint32_t)buf + len) + 0x1FU) & ~0x1FU;
    uint32_t size = (end > start) ? (end - start) : 0;
    if (size == 0) return;
    SCB_CleanDCache_by_Addr((uint32_t *)start, size);
}

void DMA_InvalidateBuffer(void *buf, uint32_t len)
{
    if (buf == NULL || len == 0) return;
    /* Align region to cache line boundaries (start down, end up) */
    uint32_t start = (uint32_t)buf & ~0x1FU;
    uint32_t end = (((uint32_t)buf + len) + 0x1FU) & ~0x1FU;
    uint32_t size = (end > start) ? (end - start) : 0;
    if (size == 0) return;
    SCB_InvalidateDCache_by_Addr((uint32_t *)start, size);
}

void DMA_RegisterErrorCallback(dma_error_cb_t cb)
{
    __disable_irq();
    s_err_cb = cb;
    __enable_irq();
}

/* HAL 全局 DMA 错误回调钩子，由 HAL 在 DMA 错误发生时调用 */
void HAL_DMA_ErrorCallback(DMA_HandleTypeDef *hdma)
{
    /* 更新全局计数与最近错误句柄 */
    __disable_irq();
    s_dma_error_count++;
    s_last_error_hdma = hdma;
    __enable_irq();

    /* 调用注册的用户回调（若有） */
    if (s_err_cb)
    {
        s_err_cb(hdma);
    }
}

uint32_t DMA_GetErrorCount(void)
{
    return s_dma_error_count;
}

void DMA_ResetErrorCount(void)
{
    __disable_irq();
    s_dma_error_count = 0;
    s_last_error_hdma = NULL;
    __enable_irq();
}

DMA_HandleTypeDef* DMA_GetLastErrorHandle(void)
{
    return (DMA_HandleTypeDef*)s_last_error_hdma;
}
