#include "drv_dev/acs712_driver.h"

#define ADC12_RESOLUTION    4096.0f   /* 12-bit ADC */

void ACS712_Init(acs712_dev_t *dev, float zero_volts, float sens, float ref_volts)
{
    dev->zero_offset = zero_volts;
    dev->sensitivity = sens;
    dev->adc_ref     = ref_volts;
    dev->adc_raw     = 0;
}

float ACS712_ReadCurrent(const acs712_dev_t *dev, const uint32_t *adc_buf, int channel)
{
    uint32_t raw = adc_buf[channel];
    float voltage = ((float)raw * dev->adc_ref) / ADC12_RESOLUTION;
    float current = (voltage - dev->zero_offset) / dev->sensitivity;
    return current;
}
