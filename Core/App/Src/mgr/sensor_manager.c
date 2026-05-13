#include "mgr/sensor_manager.h"
#include "drv_dev/sht30_driver.h"
#include "drv_dev/lps22df_driver.h"
#include "drv_dev/lsm6dsv16x_driver.h"
#include "drv_dev/ssd1306_driver.h"
#include "stm32h7xx_hal.h"

/* ── 设备实例（静态，模块内部持有）───────────────── */
static lps22df_dev_t    s_lps22df;
static lsm6dsv16x_dev_t s_lsm6dsv16x;

/* ── I2C 地址 ────────────────────────────────────── */
#define ADDR_SHT30      0x44
#define ADDR_LPS22DF    0x5C
#define ADDR_LSM6DSV16X 0x6A
#define ADDR_SSD1306    0x3C

/* ── 初始化 ──────────────────────────────────────── */
uint8_t SensorManager_Init(void)
{
    uint8_t ok_mask = 0;

    /* SHT30 温湿度传感器 */
    if (SHT30_Init(ADDR_SHT30) == 0) {
        ok_mask |= SENSOR_OK_SHT30;
    }

    /* LPS22DF 气压传感器，ODR=10Hz */
    if (LPS22DF_Init(&s_lps22df, ADDR_LPS22DF, LPS_ODR_10HZ) == 0) {
        ok_mask |= SENSOR_OK_LPS22DF;
    }

    /* LSM6DSV16X 6 轴 IMU，±4g / ±500dps / ODR=104Hz */
    if (LSM6DSV16X_Init(&s_lsm6dsv16x, ADDR_LSM6DSV16X,
                         ACCEL_FS_4G, GYRO_FS_500, ODR_104HZ) == 0) {
        ok_mask |= SENSOR_OK_LSM6DSV16X;
    }

    /* SSD1306 OLED（仅初始化，不参与周期采样）*/
    if (SSD1306_Init(ADDR_SSD1306) == 0) {
        ok_mask |= SENSOR_OK_SSD1306;
    }

    return ok_mask;
}

/* ── 全量读取 ────────────────────────────────────── */
uint8_t SensorManager_ReadAll(all_sensor_data_t *data)
{
    uint8_t ok_mask = 0;

    if (data == NULL) return 0;

    data->timestamp_ms = HAL_GetTick();

    /* SHT30 — 温湿度，阻塞 ~20ms */
    {
        sht30_data_t sht;
        if (SHT30_ReadData(ADDR_SHT30, &sht) == 0) {
            data->temperature = sht.temperature;
            data->humidity    = sht.humidity;
            ok_mask |= SENSOR_OK_SHT30;
        }
    }

    /* LPS22DF — 气压，阻塞 ~1ms */
    {
        lps22df_data_t lps;
        if (LPS22DF_ReadData(&s_lps22df, &lps) == 0) {
            data->pressure = lps.pressure;
            ok_mask |= SENSOR_OK_LPS22DF;
        }
    }

    /* LSM6DSV16X — 加速度 + 陀螺仪，阻塞 ~1ms */
    {
        lsm6dsv16x_accel_t acc;
        lsm6dsv16x_gyro_t  gyr;
        uint8_t ready = 0;

        if (LSM6DSV16X_CheckDataReady(&s_lsm6dsv16x, &ready) == 0) {
            if (ready & 0x04) {  /* BIT(2) = 加速度就绪 */
                if (LSM6DSV16X_ReadAccel(&s_lsm6dsv16x, &acc) == 0) {
                    data->accel_x = acc.x;
                    data->accel_y = acc.y;
                    data->accel_z = acc.z;
                }
            }
            if (ready & 0x02) {  /* BIT(1) = 陀螺仪就绪 */
                if (LSM6DSV16X_ReadGyro(&s_lsm6dsv16x, &gyr) == 0) {
                    data->gyro_x = gyr.x;
                    data->gyro_y = gyr.y;
                    data->gyro_z = gyr.z;
                }
            }
            ok_mask |= SENSOR_OK_LSM6DSV16X;
        }
    }

    return ok_mask;
}
