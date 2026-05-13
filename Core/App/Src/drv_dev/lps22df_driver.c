#include "drv_dev/lps22df_driver.h"
#include "drv_hal/i2c_driver.h"

/* 寄存器 */
#define REG_WHO_AM_I        0x0F
#define REG_CTRL_REG1       0x10
#define REG_CTRL_REG2       0x11
#define REG_STATUS          0x27
#define REG_PRESS_OUT_XL    0x28
#define REG_PRESS_OUT_L     0x29
#define REG_PRESS_OUT_H     0x2A
#define REG_TEMP_OUT_L      0x2C
#define REG_TEMP_OUT_H      0x2D

#define WHO_AM_I_EXPECTED   0xB4

#define LPS_TIMEOUT_MS      50

static int reg_read(const lps22df_dev_t *dev, uint8_t reg, uint8_t *val)
{
    return I2C_Master_TransmitBlocking(dev->addr, &reg, 1, LPS_TIMEOUT_MS) == 0
        && I2C_Master_ReceiveBlocking(dev->addr, val, 1, LPS_TIMEOUT_MS) == 0 ? 0 : -1;
}

static int reg_write(const lps22df_dev_t *dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[] = { reg, val };
    return I2C_Master_TransmitBlocking(dev->addr, buf, 2, LPS_TIMEOUT_MS);
}

static int reg_burst_read(const lps22df_dev_t *dev, uint8_t reg, uint8_t *buf, int len)
{
    if (I2C_Master_TransmitBlocking(dev->addr, &reg, 1, LPS_TIMEOUT_MS) != 0)
        return -1;
    return I2C_Master_ReceiveBlocking(dev->addr, buf, len, LPS_TIMEOUT_MS);
}

int LPS22DF_Init(lps22df_dev_t *dev, uint16_t addr, lps22df_odr_t odr)
{
    dev->addr = addr;

    uint8_t whoami = 0;
    if (reg_read(dev, REG_WHO_AM_I, &whoami) != 0) return -1;
    if (whoami != WHO_AM_I_EXPECTED) return -2;

    /* 软件复位 + BDU */
    reg_write(dev, REG_CTRL_REG2, 0x04); /* SWRESET */
    HAL_Delay(5);

    reg_write(dev, REG_CTRL_REG2, 0x02); /* BDU: block data update */

    /* ODR[7:4] + AVG[3:1]=010(16次) */
    uint8_t ctrl1 = ((uint8_t)odr << 4) | 0x04;
    reg_write(dev, REG_CTRL_REG1, ctrl1);

    return 0;
}

int LPS22DF_ReadData(const lps22df_dev_t *dev, lps22df_data_t *data)
{
    uint8_t buf[6];

    /* 读取气压（3 字节 0x28-0x2A）+ 1 字节保留(0x2B) + 温度（2 字节 0x2C-0x2D）*/
    if (reg_burst_read(dev, REG_PRESS_OUT_XL, buf, 6) != 0) return -1;

    /* 24-bit 气压（2's complement） */
    int32_t raw_press = ((int32_t)buf[2] << 16) | ((int32_t)buf[1] << 8) | buf[0];
    if (raw_press & 0x00800000) raw_press |= 0xFF000000; /* 符号扩展 */

    data->pressure = (float)raw_press / 4096.0f;  /* hPa */

    /* 16-bit 温度 */
    int16_t raw_temp = ((int16_t)buf[5] << 8) | buf[4];
    data->temperature = (float)raw_temp / 100.0f;  /* °C */

    return 0;
}
