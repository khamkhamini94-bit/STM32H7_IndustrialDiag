#ifndef SHT30_DRIVER_H
#define SHT30_DRIVER_H

#include <stdint.h>

/* SHT30 I2C 地址（ADDR 引脚电平选择） */
#define SHT30_ADDR_LOW    0x44
#define SHT30_ADDR_HIGH   0x45

/* 单次测量命令（高重复性 + 时钟拉伸） */
#define SHT30_CMD_SINGLE_HIGH_REP    0x2C06
#define SHT30_CMD_SINGLE_MED_REP     0x2C0D
#define SHT30_CMD_SINGLE_LOW_REP     0x2C10

/* 状态寄存器 */
#define SHT30_CMD_READ_STATUS        0xF32D
#define SHT30_CMD_CLEAR_STATUS       0x3041

/* 软件复位 */
#define SHT30_CMD_SOFT_RESET         0x30A2

typedef struct {
    float temperature;   /* °C */
    float humidity;      /* %RH */
} sht30_data_t;

/**
 * 初始化 SHT30（发送软复位，等待传感器就绪）
 * @param addr I2C 地址，通常为 SHT30_ADDR_LOW
 * @return 0 成功，非 0 失败
 */
int SHT30_Init(uint16_t addr);

/**
 * 执行单次测量并读取温湿度
 * @param addr I2C 地址
 * @param data 输出温湿度数据
 * @return 0 成功，非 0 失败
 */
int SHT30_ReadData(uint16_t addr, sht30_data_t *data);

#endif
