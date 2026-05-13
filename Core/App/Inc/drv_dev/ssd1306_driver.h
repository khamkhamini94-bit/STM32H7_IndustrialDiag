#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#include <stdint.h>

#define SSD1306_ADDR    0x3C
#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64
#define SSD1306_PAGES   8        /* 64/8 */

/**
 * 初始化 SSD1306
 * @param addr I2C 地址，通常 0x3C
 * @return 0 成功，非 0 失败
 */
int SSD1306_Init(uint16_t addr);

/**
 * 清空帧缓冲区（不刷新到屏幕）
 */
void SSD1306_Clear(void);

/**
 * 将帧缓冲区写入屏幕
 */
int SSD1306_Refresh(uint16_t addr);

/**
 * 设置像素点（写入帧缓冲区，不刷新）
 */
void SSD1306_SetPixel(uint8_t x, uint8_t y, uint8_t on);

/**
 * 在指定位置显示字符串（5x7 字体，自动换行）
 * @param x 列 (0-127)
 * @param y 页 (0-7, 每页 8 像素)
 * @param str 字符串
 */
void SSD1306_ShowString(uint8_t x, uint8_t y, const char *str);

/**
 * 显示有符号整数
 */
void SSD1306_ShowInt(uint8_t x, uint8_t y, int32_t val);

/**
 * 显示浮点数
 */
void SSD1306_ShowFloat(uint8_t x, uint8_t y, float val, int decimals);

/**
 * 填充一整页（用于绘制进度条）
 * @param page 页号 (0-7)
 * @param ratio 填充比例 (0-100)
 */
void SSD1306_FillPage(uint8_t page, uint8_t ratio);

#endif
