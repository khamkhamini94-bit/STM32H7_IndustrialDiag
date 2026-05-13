#include "drv_dev/lsm6dsv16x_driver.h"
#include "drv_hal/i2c_driver.h"

/* 寄存器地址 */
#define REG_FUNC_CFG_ACCESS     0x01
#define REG_PIN_CTRL            0x02
#define REG_WHO_AM_I            0x0F
#define REG_CTRL1_XL            0x10
#define REG_CTRL2_G             0x11
#define REG_CTRL3_C             0x12
#define REG_CTRL4_C             0x13
#define REG_CTRL5_C             0x14
#define REG_CTRL6_C             0x15
#define REG_CTRL7_G             0x16
#define REG_CTRL8_XL            0x17
#define REG_CTRL9_XL            0x18
#define REG_CTRL10_C            0x19
#define REG_STATUS_REG          0x1E
#define REG_OUT_TEMP_L          0x20
#define REG_OUT_TEMP_H          0x21
#define REG_OUTX_L_G            0x22
#define REG_OUTX_H_G            0x23
#define REG_OUTY_L_G            0x24
#define REG_OUTY_H_G            0x25
#define REG_OUTZ_L_G            0x26
#define REG_OUTZ_H_G            0x27
#define REG_OUTX_L_A            0x28
#define REG_OUTX_H_A            0x29
#define REG_OUTY_L_A            0x2A
#define REG_OUTY_H_A            0x2B
#define REG_OUTZ_L_A            0x2C
#define REG_OUTZ_H_A            0x2D

#define WHO_AM_I_EXPECTED       0x70

#define IMU_TIMEOUT_MS          50

/* 加速度计灵敏度 (mg/LSB)，对应 FS 2/4/8/16g */
static const float accel_sensitivity[] = {
    0.061f, 0.122f, 0.244f, 0.488f
};

/* 陀螺仪灵敏度 (mdps/LSB)，对应 FS 125/250/500/1000/2000/4000 */
static const float gyro_sensitivity[] = {
    4.375f, 8.75f, 17.5f, 35.0f, 70.0f, 140.0f
};

static int reg_read(const lsm6dsv16x_dev_t *dev, uint8_t reg, uint8_t *val)
{
    return I2C_Master_TransmitBlocking(dev->addr, &reg, 1, IMU_TIMEOUT_MS) == 0
        && I2C_Master_ReceiveBlocking(dev->addr, val, 1, IMU_TIMEOUT_MS) == 0 ? 0 : -1;
}

static int reg_write(const lsm6dsv16x_dev_t *dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[] = { reg, val };
    return I2C_Master_TransmitBlocking(dev->addr, buf, 2, IMU_TIMEOUT_MS);
}

static int reg_burst_read(const lsm6dsv16x_dev_t *dev, uint8_t reg, uint8_t *buf, int len)
{
    if (I2C_Master_TransmitBlocking(dev->addr, &reg, 1, IMU_TIMEOUT_MS) != 0)
        return -1;
    return I2C_Master_ReceiveBlocking(dev->addr, buf, len, IMU_TIMEOUT_MS);
}

int LSM6DSV16X_Init(lsm6dsv16x_dev_t *dev, uint16_t addr,
                    lsm6dsv16x_accel_fs_t afs, lsm6dsv16x_gyro_fs_t gfs,
                    lsm6dsv16x_odr_t odr)
{
    dev->addr      = addr;
    dev->accel_fs  = afs;
    dev->gyro_fs   = gfs;

    uint8_t whoami = 0;
    if (reg_read(dev, REG_WHO_AM_I, &whoami) != 0) return -1;
    if (whoami != WHO_AM_I_EXPECTED) return -2;

    /* 软件复位 */
    reg_write(dev, REG_CTRL3_C, 0x01);   /* SW_RESET */
    HAL_Delay(10);

    /* 禁用 I3C，启用 BDU（阻塞数据更新），自动地址递增 */
    reg_write(dev, REG_CTRL9_XL, 0x02);  /* I3C disable */
    reg_write(dev, REG_CTRL3_C, 0x44);   /* BDU + IF_INC */

    /* 加速度计：ODR + FS */
    uint8_t ctrl1 = ((uint8_t)odr << 4) | ((uint8_t)afs << 2);
    reg_write(dev, REG_CTRL1_XL, ctrl1);

    /* 陀螺仪：ODR[7:4] + FS_G[3:1] + FS_125[0]
     * FS_G 位映射：125/250 → 000, 500 → 001, 1000 → 010, 2000 → 011, 4000 → 100
     * FS_125 仅在 ±125dps 时置 1 */
    uint8_t ctrl2 = ((uint8_t)odr << 4);
    if (gfs == GYRO_FS_125) {
        ctrl2 |= 0x01;                     /* FS_G=000, FS_125=1 */
    } else if (gfs == GYRO_FS_250) {
        ctrl2 |= 0x00;                     /* FS_G=000, FS_125=0 */
    } else {
        ctrl2 |= ((uint8_t)(gfs - 1) << 1); /* FS_G = gfs-1, FS_125=0 */
    }
    reg_write(dev, REG_CTRL2_G, ctrl2);

    /* 加速度计滤波：LPF2 使能，BW = ODR/4 */
    reg_write(dev, REG_CTRL8_XL, 0x02);

    return 0;
}

int LSM6DSV16X_ReadAccel(const lsm6dsv16x_dev_t *dev, lsm6dsv16x_accel_t *data)
{
    uint8_t raw[6];
    if (reg_burst_read(dev, REG_OUTX_L_A, raw, 6) != 0) return -1;

    int16_t ax = (int16_t)((raw[1] << 8) | raw[0]);
    int16_t ay = (int16_t)((raw[3] << 8) | raw[2]);
    int16_t az = (int16_t)((raw[5] << 8) | raw[4]);

    float sens = accel_sensitivity[dev->accel_fs] / 1000.0f; /* mg/LSB → g/LSB */
    data->x = ax * sens;
    data->y = ay * sens;
    data->z = az * sens;
    return 0;
}

int LSM6DSV16X_ReadGyro(const lsm6dsv16x_dev_t *dev, lsm6dsv16x_gyro_t *data)
{
    uint8_t raw[6];
    if (reg_burst_read(dev, REG_OUTX_L_G, raw, 6) != 0) return -1;

    int16_t gx = (int16_t)((raw[1] << 8) | raw[0]);
    int16_t gy = (int16_t)((raw[3] << 8) | raw[2]);
    int16_t gz = (int16_t)((raw[5] << 8) | raw[4]);

    float sens = gyro_sensitivity[dev->gyro_fs] / 1000.0f; /* mdps/LSB → dps/LSB */
    data->x = gx * sens;
    data->y = gy * sens;
    data->z = gz * sens;
    return 0;
}

float LSM6DSV16X_ReadTemp(const lsm6dsv16x_dev_t *dev)
{
    uint8_t raw[2];
    if (reg_burst_read(dev, REG_OUT_TEMP_L, raw, 2) != 0) return -273.0f;

    int16_t t = (int16_t)((raw[1] << 8) | raw[0]);
    return 25.0f + (float)t / 256.0f;
}

int LSM6DSV16X_CheckDataReady(const lsm6dsv16x_dev_t *dev, uint8_t *mask)
{
    return reg_read(dev, REG_STATUS_REG, mask);
}
