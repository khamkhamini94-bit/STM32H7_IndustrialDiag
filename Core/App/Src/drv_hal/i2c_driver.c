/* i2c_driver.c - Simple I2C master blocking implementation */
#include "drv_hal/i2c_driver.h"
#include "stm32h7xx_hal.h"

/* extern handle from generated code */
extern I2C_HandleTypeDef hi2c1;

static volatile i2c_err_cb_t s_i2c_err_cb = NULL;
static volatile i2c_tx_cb_t s_i2c_tx_cb = NULL;
static volatile i2c_rx_cb_t s_i2c_rx_cb = NULL;

int I2C_Master_Transmit_IT(uint16_t dev_addr, uint8_t *data, uint16_t size)
{
    if (data == NULL || size == 0) return -1;
    if (HAL_I2C_Master_Transmit_IT(&hi2c1, dev_addr, data, size) != HAL_OK)
    {
        if (s_i2c_err_cb) s_i2c_err_cb(&hi2c1, hi2c1.ErrorCode ? HAL_ERROR : HAL_BUSY);
        return -1;
    }
    return 0;
}

int I2C_Master_Receive_IT(uint16_t dev_addr, uint8_t *data, uint16_t size)
{
    if (data == NULL || size == 0) return -1;
    if (HAL_I2C_Master_Receive_IT(&hi2c1, dev_addr, data, size) != HAL_OK)
    {
        if (s_i2c_err_cb) s_i2c_err_cb(&hi2c1, hi2c1.ErrorCode ? HAL_ERROR : HAL_BUSY);
        return -1;
    }
    return 0;
}

void I2C_RegisterTxCallback(i2c_tx_cb_t cb)
{
    __disable_irq(); s_i2c_tx_cb = cb; __enable_irq();
}

void I2C_RegisterRxCallback(i2c_rx_cb_t cb)
{
    __disable_irq(); s_i2c_rx_cb = cb; __enable_irq();
}

/* HAL callback routers */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == &hi2c1)
    {
        if (s_i2c_tx_cb) s_i2c_tx_cb(hi2c);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == &hi2c1)
    {
        if (s_i2c_rx_cb) s_i2c_rx_cb(hi2c);
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == &hi2c1)
    {
        if (s_i2c_err_cb) s_i2c_err_cb(hi2c, HAL_ERROR);
    }
}

void I2C_Driver_Init(void)
{
    /* No-op placeholder. Ensure MX_I2C1_Init() is called before using the driver. */
}

int I2C_Master_TransmitBlocking(uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout_ms)
{
    if (data == NULL || size == 0) return -1;
    if (HAL_I2C_Master_Transmit(&hi2c1, dev_addr, data, size, timeout_ms) != HAL_OK)
    {
        if (s_i2c_err_cb) s_i2c_err_cb(&hi2c1, hi2c1.ErrorCode ? HAL_ERROR : HAL_BUSY);
        return -1;
    }
    return 0;
}

int I2C_Master_ReceiveBlocking(uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout_ms)
{
    if (data == NULL || size == 0) return -1;
    if (HAL_I2C_Master_Receive(&hi2c1, dev_addr, data, size, timeout_ms) != HAL_OK)
    {
        if (s_i2c_err_cb) s_i2c_err_cb(&hi2c1, hi2c1.ErrorCode ? HAL_ERROR : HAL_BUSY);
        return -1;
    }
    return 0;
}

void I2C_RegisterErrorCallback(i2c_err_cb_t cb)
{
    __disable_irq();
    s_i2c_err_cb = cb;
    __enable_irq();
}
