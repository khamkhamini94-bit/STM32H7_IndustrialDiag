#ifndef LPS22DF_DRIVER_H
#define LPS22DF_DRIVER_H

#include <stdint.h>

/* I2C 地址 */
#define LPS22DF_ADDR_LOW    0x5C   /* SDO/SA0 = GND */
#define LPS22DF_ADDR_HIGH   0x5D   /* SDO/SA0 = VDD */

/* 输出数据速率 */
typedef enum {
    LPS_ODR_ONE_SHOT = 0x0,
    LPS_ODR_1HZ      = 0x1,
    LPS_ODR_4HZ      = 0x2,
    LPS_ODR_10HZ     = 0x3,
    LPS_ODR_25HZ     = 0x4,
    LPS_ODR_50HZ     = 0x5,
    LPS_ODR_75HZ     = 0x6,
    LPS_ODR_100HZ    = 0x7,
    LPS_ODR_200HZ    = 0x8,
} lps22df_odr_t;

typedef struct {
    float pressure;    /* hPa */
    float temperature; /* °C */
} lps22df_data_t;

typedef struct {
    uint16_t addr;
} lps22df_dev_t;

/**
 * 初始化 LPS22DF
 * @param addr I2C 地址
 * @param odr  输出数据速率
 * @return 0 成功，非 0 失败
 */
int LPS22DF_Init(lps22df_dev_t *dev, uint16_t addr, lps22df_odr_t odr);

/**
 * 读取压力和温度
 * @return 0 成功，非 0 失败
 */
int LPS22DF_ReadData(const lps22df_dev_t *dev, lps22df_data_t *data);

#endif
