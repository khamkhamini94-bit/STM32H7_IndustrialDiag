#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdint.h>

/* ── 统一传感器数据结构 ─────────────────────────── */
typedef struct {
    /* SHT30 */
    float temperature;      /* °C  */
    float humidity;         /* %RH */

    /* LPS22DF */
    float pressure;         /* hPa */

    /* LSM6DSV16X */
    float accel_x, accel_y, accel_z;  /* g   */
    float gyro_x,  gyro_y,  gyro_z;   /* dps */

    uint32_t timestamp_ms;
} all_sensor_data_t;

/* ── 各传感器状态位 ─────────────────────────────── */
#define SENSOR_OK_SHT30       (1u << 0)
#define SENSOR_OK_LPS22DF     (1u << 1)
#define SENSOR_OK_LSM6DSV16X  (1u << 2)
#define SENSOR_OK_SSD1306     (1u << 3)

/* ── API ────────────────────────────────────────── */
/**
 * 初始化全部 I2C 传感器（SHT30 / LPS22DF / LSM6DSV16X / SSD1306）
 * @return 位掩码，各 bit 对应 SENSOR_OK_xxx，表示哪些传感器初始化成功
 */
uint8_t SensorManager_Init(void);

/**
 * 一次性读取全部传感器数据
 * @param data 输出数据
 * @return 位掩码，表示哪些传感器本次读取成功
 */
uint8_t SensorManager_ReadAll(all_sensor_data_t *data);

#endif /* SENSOR_MANAGER_H */
