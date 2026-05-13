/* i2c_driver.h - Simple I2C master blocking API and placeholders for IRQ/DMA */
#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "i2c.h"

typedef void (*i2c_err_cb_t)(I2C_HandleTypeDef *hi2c, HAL_StatusTypeDef status);
typedef void (*i2c_tx_cb_t)(I2C_HandleTypeDef *hi2c);
typedef void (*i2c_rx_cb_t)(I2C_HandleTypeDef *hi2c);

/* Initialize driver (call after MX_I2C1_Init if used) */
void I2C_Driver_Init(void);

/* Master blocking transfer (wraps HAL_I2C_Master_Transmit/Receive).
   Returns 0 on success, -1 on error. */
int I2C_Master_TransmitBlocking(uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout_ms);
int I2C_Master_ReceiveBlocking(uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout_ms);

/* Optional: placeholders for IRQ/DMA-based APIs (not implemented yet) */
int I2C_Master_Transmit_IT(uint16_t dev_addr, uint8_t *data, uint16_t size);
int I2C_Master_Receive_IT(uint16_t dev_addr, uint8_t *data, uint16_t size);

/* Register callbacks for non-blocking transfers */
void I2C_RegisterTxCallback(i2c_tx_cb_t cb);
void I2C_RegisterRxCallback(i2c_rx_cb_t cb);

/* Register error callback */
void I2C_RegisterErrorCallback(i2c_err_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif // I2C_DRIVER_H
