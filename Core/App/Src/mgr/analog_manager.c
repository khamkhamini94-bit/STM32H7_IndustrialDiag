#include "mgr/analog_manager.h"
#include "drv_hal/adc_driver.h"
#include "drv_dev/acs712_driver.h"
#include "drv_hal/tim_driver.h"
#include "stm32h7xx_hal.h"

/* ── 硬件参数（可通过 Set 接口修改）──────────────── */
#define ADC_BUF_SIZE        16      /* DMA 循环缓冲（足够 5 次扫描）*/
#define ACS712_CHANNEL       0      /* ADC 通道索引：CH3=0, CH4=1, CH5=2 */
#define ACS712_ZERO_V        1.65f  /* 零电流电压 (V)         */
#define ACS712_SENSITIVITY   0.122f /* 灵敏度 (V/A), ACS712-05B @3.3V */
#define ADC_REF_V            3.30f  /* ADC 参考电压 (V)       */
#define ENCODER_PPR          1024   /* 编码器每转脉冲数        */

/* ── 静态实例 ───────────────────────────────────── */
static acs712_dev_t s_acs712;
static uint32_t     s_adc_buf[ADC_BUF_SIZE];
static uint32_t     s_encoder_ppr = ENCODER_PPR;
static float        s_current_offset = ACS712_ZERO_V;

/* ── 初始化 ──────────────────────────────────────── */
uint8_t AnalogManager_Init(void)
{
    uint8_t ok_mask = 0;

    /* ADC 驱动状态检查 */
    if (ADC_Driver_GetStatus() >= 0) {
        ok_mask |= ANALOG_OK_ADC;
    }

    /* ACS712 校准参数 */
    ACS712_Init(&s_acs712, ACS712_ZERO_V, ACS712_SENSITIVITY, ADC_REF_V);
    ok_mask |= ANALOG_OK_CURRENT;

    /* TIM3 编码器（HAL 层已在 MX_TIM3_Init 中初始化）*/
    ok_mask |= ANALOG_OK_ENCODER;

    return ok_mask;
}

/* ── 启动采样 ────────────────────────────────────── */
int AnalogManager_Start(void)
{
    int ret = ADC_Driver_StartDMA(s_adc_buf, ADC_BUF_SIZE);
    if (ret != 0) return ret;
    return TIM3_Encoder_Start();
}

/* ── 停止采样 ────────────────────────────────────── */
void AnalogManager_Stop(void)
{
    ADC_Driver_StopDMA();
    TIM3_Encoder_Stop();
}

/* ── 读取全部模拟量 ──────────────────────────────── */
uint8_t AnalogManager_ReadAll(analog_data_t *data)
{
    uint8_t ok_mask = 0;

    if (data == NULL) return 0;

    data->timestamp_ms = HAL_GetTick();

    /* ADC 原始值（取 DMA 缓冲区最近一次扫描的前 3 通道）*/
    data->adc_raw[0] = (uint16_t)(s_adc_buf[0] & 0xFFFF);
    data->adc_raw[1] = (uint16_t)(s_adc_buf[1] & 0xFFFF);
    data->adc_raw[2] = (uint16_t)(s_adc_buf[2] & 0xFFFF);
    ok_mask |= ANALOG_OK_ADC;

    /* ACS712 电流 */
    data->current = ACS712_ReadCurrent(&s_acs712, s_adc_buf, ACS712_CHANNEL);
    ok_mask |= ANALOG_OK_CURRENT;

    /* 编码器 */
    data->encoder_count = (int32_t)TIM3_GetCounter();
    data->encoder_rpm   = TIM3_GetRPM(s_encoder_ppr);
    ok_mask |= ANALOG_OK_ENCODER;

    return ok_mask;
}

/* ── 参数配置 ────────────────────────────────────── */
void AnalogManager_SetCurrentOffset(float zero_v)
{
    s_current_offset = zero_v;
    ACS712_Init(&s_acs712, zero_v, ACS712_SENSITIVITY, ADC_REF_V);
}

void AnalogManager_SetEncoderPPR(uint32_t ppr)
{
    s_encoder_ppr = ppr;
}
