#ifndef LSM6DSV16X_DRIVER_H
#define LSM6DSV16X_DRIVER_H

#include <stdint.h>

/* I2C 地址 */
#define LSM6DSV16X_ADDR_LOW    0x6A   /* SDO/SA0 = GND */
#define LSM6DSV16X_ADDR_HIGH   0x6B   /* SDO/SA0 = VDD */

/* 加速度计量程 */
typedef enum {
    ACCEL_FS_2G  = 0,   /* ±2g  */
    ACCEL_FS_4G  = 1,   /* ±4g  */
    ACCEL_FS_8G  = 2,   /* ±8g  */
    ACCEL_FS_16G = 3    /* ±16g */
} lsm6dsv16x_accel_fs_t;

/* 陀螺仪量程 */
typedef enum {
    GYRO_FS_125  = 0,   /* ±125dps  */
    GYRO_FS_250  = 1,   /* ±250dps  */
    GYRO_FS_500  = 2,   /* ±500dps  */
    GYRO_FS_1000 = 3,   /* ±1000dps */
    GYRO_FS_2000 = 4,   /* ±2000dps */
    GYRO_FS_4000 = 5    /* ±4000dps */
} lsm6dsv16x_gyro_fs_t;

/* 输出数据速率 */
typedef enum {
    ODR_OFF    = 0x0,
    ODR_1_6HZ  = 0xB,
    ODR_12_5HZ = 0x1,
    ODR_26HZ   = 0x2,
    ODR_52HZ   = 0x3,
    ODR_104HZ  = 0x4,
    ODR_208HZ  = 0x5,
    ODR_416HZ  = 0x6,
    ODR_833HZ  = 0x7,
    ODR_1667HZ  = 0x8,
    ODR_3333HZ  = 0x9,
    ODR_6667HZ  = 0xA,
} lsm6dsv16x_odr_t;

/* 加速度计数据 */
typedef struct {
    float x, y, z;  /* g */
} lsm6dsv16x_accel_t;

/* 陀螺仪数据 */
typedef struct {
    float x, y, z;  /* dps */
} lsm6dsv16x_gyro_t;

/* 设备句柄 */
typedef struct {
    uint16_t addr;
    lsm6dsv16x_accel_fs_t accel_fs;
    lsm6dsv16x_gyro_fs_t  gyro_fs;
} lsm6dsv16x_dev_t;

/**
 * 初始化 LSM6DSV16X
 * @param dev      设备句柄
 * @param addr     I2C 地址
 * @param afs     加速度量程
 * @param gfs     陀螺仪量程
 * @param odr     输出数据速率
 * @return 0 成功，非 0 检查 WHO_AM_I 失败
 */
int LSM6DSV16X_Init(lsm6dsv16x_dev_t *dev, uint16_t addr,
                    lsm6dsv16x_accel_fs_t afs, lsm6dsv16x_gyro_fs_t gfs,
                    lsm6dsv16x_odr_t odr);

/**
 * 读取加速度计数据
 * @return 0 成功，非 0 失败
 */
int LSM6DSV16X_ReadAccel(const lsm6dsv16x_dev_t *dev, lsm6dsv16x_accel_t *data);

/**
 * 读取陀螺仪数据
 * @return 0 成功，非 0 失败
 */
int LSM6DSV16X_ReadGyro(const lsm6dsv16x_dev_t *dev, lsm6dsv16x_gyro_t *data);

/**
 * 读取温度
 * @return 温度 (°C)
 */
float LSM6DSV16X_ReadTemp(const lsm6dsv16x_dev_t *dev);

/**
 * 检查数据是否就绪
 * @param mask  BIT(0)=温度, BIT(1)=陀螺, BIT(2)=加速度
 * @return 0 成功读取状态，非 0 失败
 */
int LSM6DSV16X_CheckDataReady(const lsm6dsv16x_dev_t *dev, uint8_t *mask);

#endif
