#include "mgr/comm_manager.h"
#include "drv_dev/esp8266_driver.h"
#include "drv_hal/usart_driver.h"
#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>

/* ── HMI RX 缓冲区 ────────────────────────────────── */
#define HMI_RX_BUF_SIZE   128
#define HMI_FRAME_LEN      6      /* 0xAA + cmd_id + param(4B) */

static uint8_t  s_hmi_rx_buf[HMI_RX_BUF_SIZE];
static hmi_cmd_cb_t s_hmi_cb = NULL;

/* ── HMI RX 待重启标志（ISR 只设标志，不在 ISR 内调 HAL）── */
static volatile uint8_t s_hmi_need_restart = 0;

/* ── HMI DMA 完成回调（在 HAL_UART_RxCpltCallback 的 ISR 上下文）─ */
static void hmi_rx_done(void)
{
    if (s_hmi_cb != NULL) {
        /* 扫描缓冲区，寻找 0xAA 帧头 */
        for (int i = 0; i <= HMI_RX_BUF_SIZE - HMI_FRAME_LEN; i++) {
            if (s_hmi_rx_buf[i] == 0xAA) {
                uint8_t  cmd_id = s_hmi_rx_buf[i + 1];
                int32_t  param  = (int32_t)(s_hmi_rx_buf[i + 2]
                                  | (s_hmi_rx_buf[i + 3] << 8)
                                  | (s_hmi_rx_buf[i + 4] << 16)
                                  | (s_hmi_rx_buf[i + 5] << 24));
                s_hmi_cb(cmd_id, param);
                i += (HMI_FRAME_LEN - 1);  /* 跳过当前帧，继续扫描 */
            }
        }
    }

    /* 不在 ISR 中直接调 HAL，设标志让应用层轮询重启 */
    s_hmi_need_restart = 1;
}

/* ── 应用层调用：必要时重启 DMA 接收 ──────────────── */
void CommManager_ServiceHMI(void)
{
    if (s_hmi_need_restart) {
        s_hmi_need_restart = 0;
        USART1_StartRxDMA(s_hmi_rx_buf, HMI_RX_BUF_SIZE);
    }
}

/* ── 初始化 ──────────────────────────────────────── */
uint8_t CommManager_Init(void)
{
    uint8_t ok_mask = 0;

    /* ESP8266 AT 初始化 */
    if (ESP8266_Init() == 0) {
        ok_mask |= 0x01;
    }

    /* 注册 USART1 RX 回调（HMI DMA 完成时触发） */
    USART1_RegisterRxCallbacks(NULL, hmi_rx_done);

    /* 首次启动 HMI DMA 接收 */
    CommManager_StartHMIReceive();
    ok_mask |= 0x02;

    return ok_mask;
}

/* ── WiFi / TCP ──────────────────────────────────── */
int CommManager_ConnectWiFi(void)
{
    return ESP8266_ConnectWiFi(ESP_WIFI_SSID, ESP_WIFI_PASSWORD);
}

int CommManager_ConnectServer(const char *ip, uint16_t port)
{
    return ESP8266_ConnectServer(ip, port);
}

void CommManager_DisconnectServer(void)
{
    ESP8266_Disconnect();
}

int CommManager_SendToServer(const uint8_t *data, uint16_t len)
{
    return ESP8266_Send(data, len);
}

comm_state_t CommManager_GetState(void)
{
    switch (ESP8266_GetState()) {
    case ESP_STATE_IDLE:             return COMM_WIFI_IDLE;
    case ESP_STATE_READY:            return COMM_WIFI_READY;
    case ESP_STATE_WIFI_CONNECTED:   return COMM_WIFI_CONNECTED;
    case ESP_STATE_SERVER_CONNECTED: return COMM_SERVER_CONNECTED;
    default:                        return COMM_ERROR;
    }
}

/* ── HMI 发送（printf → USART1） ─────────────────── */
void CommManager_SendHMI(uint8_t msg_type, uint8_t severity,
                         const char *text, uint32_t value)
{
    static const char *sev_str[] = { "INFO", "WARN", "ERROR", "FATAL" };
    const char *sev = (severity < 4) ? sev_str[severity] : "?";
    printf("[%s][%c] %s (%lu)\r\n", sev, (char)msg_type, text, (unsigned long)value);
}

/* ── HMI 接收 ────────────────────────────────────── */
void CommManager_StartHMIReceive(void)
{
    USART1_StartRxDMA(s_hmi_rx_buf, HMI_RX_BUF_SIZE);
}

void CommManager_RegisterHMICallback(hmi_cmd_cb_t cb)
{
    __disable_irq();
    s_hmi_cb = cb;
    __enable_irq();
}
