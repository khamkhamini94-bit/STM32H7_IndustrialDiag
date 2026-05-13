#include "drv_dev/sht30_driver.h"
#include "drv_hal/i2c_driver.h"

#define SHT30_TIMEOUT_MS   100

static uint8_t sht30_crc8(const uint8_t *data, int len)
{
    uint8_t crc = 0xFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? ((crc << 1) ^ 0x31) : (crc << 1);
        }
    }
    return crc;
}

int SHT30_Init(uint16_t addr)
{
    uint8_t cmd[2];

    /* 软件复位 */
    cmd[0] = (SHT30_CMD_SOFT_RESET >> 8) & 0xFF;
    cmd[1] = SHT30_CMD_SOFT_RESET & 0xFF;
    if (I2C_Master_TransmitBlocking(addr, cmd, 2, SHT30_TIMEOUT_MS) != 0) {
        return -1;
    }

    /* 等待复位完成（SHT30 最长 1ms） */
    HAL_Delay(2);

    /* 清除状态寄存器 */
    cmd[0] = (SHT30_CMD_CLEAR_STATUS >> 8) & 0xFF;
    cmd[1] = SHT30_CMD_CLEAR_STATUS & 0xFF;
    if (I2C_Master_TransmitBlocking(addr, cmd, 2, SHT30_TIMEOUT_MS) != 0) {
        return -2;
    }

    return 0;
}

int SHT30_ReadData(uint16_t addr, sht30_data_t *data)
{
    if (data == NULL) return -1;

    uint8_t cmd[2];
    uint8_t buf[6];

    /* 发送单次测量命令（高重复性 + 时钟拉伸） */
    cmd[0] = (SHT30_CMD_SINGLE_HIGH_REP >> 8) & 0xFF;
    cmd[1] = SHT30_CMD_SINGLE_HIGH_REP & 0xFF;
    if (I2C_Master_TransmitBlocking(addr, cmd, 2, SHT30_TIMEOUT_MS) != 0) {
        return -2;
    }

    /* 读取 6 字节：Temp[15:8], Temp[7:0], CRC, Hum[15:8], Hum[7:0], CRC */
    if (I2C_Master_ReceiveBlocking(addr, buf, 6, SHT30_TIMEOUT_MS) != 0) {
        return -3;
    }

    /* CRC 校验 */
    if (sht30_crc8(buf, 2) != buf[2]) return -4;
    if (sht30_crc8(buf + 3, 2) != buf[5]) return -5;

    /* 原始值 */
    uint16_t raw_temp = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t raw_humi = ((uint16_t)buf[3] << 8) | buf[4];

    /* SHT30 数据手册公式 */
    data->temperature = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);
    data->humidity    = 100.0f * ((float)raw_humi / 65535.0f);

    return 0;
}
