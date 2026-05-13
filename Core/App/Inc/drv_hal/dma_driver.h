#ifndef DMA_DRIVER_H
#define DMA_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"

typedef void (*dma_error_cb_t)(DMA_HandleTypeDef *hdma);

/** 初始化 DMA 管理器（目前为占位） */
void DMA_Manager_Init(void);

/**
 * 清理写入到内存区域的 D-Cache，确保 DMA 之后 CPU 可见
 * @param buf 起始地址
 * @param len 字节长度
 */
void DMA_CleanBuffer(void *buf, uint32_t len);

/**
 * 失效 D-Cache 中对应内存区域的条目，保证 CPU 读取到 DMA 最新写入的数据
 */
void DMA_InvalidateBuffer(void *buf, uint32_t len);

/** 注册 DMA 错误回调（在 HAL 的 DMA 错误回调中调用） */
void DMA_RegisterErrorCallback(dma_error_cb_t cb);

/** 错误统计与查询接口 */
/** 返回自系统启动以来记录的 DMA 错误总数（全局计数） */
uint32_t DMA_GetErrorCount(void);
/** 重置 DMA 错误计数（原子操作） */
void DMA_ResetErrorCount(void);
/** 返回最近一次出错的 DMA_Handle（如果没有则返回 NULL） */
DMA_HandleTypeDef* DMA_GetLastErrorHandle(void);

#ifdef __cplusplus
}
#endif

#endif // DMA_DRIVER_H
