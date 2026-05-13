#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

#include <stdint.h>

/* ── ESP8266 状态 ────────────────────────────────── */
typedef enum {
    COMM_WIFI_IDLE = 0,
    COMM_WIFI_READY,
    COMM_WIFI_CONNECTED,
    COMM_SERVER_CONNECTED,
    COMM_ERROR
} comm_state_t;

/* ── HMI 指令回调 ────────────────────────────────── */
typedef void (*hmi_cmd_cb_t)(uint8_t cmd_id, int32_t param);

/* ── HMI 消息类型 ────────────────────────────────── */
#define HMI_MSG_STATUS    'S'
#define HMI_MSG_DATA      'D'
#define HMI_MSG_ERROR     'E'
#define HMI_MSG_CMD_ECHO  'C'

/* ── API ────────────────────────────────────────── */

/** 初始化 ESP8266 + USART1 HMI DMA 接收 */
uint8_t CommManager_Init(void);

/** 连接 WiFi（使用 esp8266_driver.h 中配置的 SSID/PW） */
int CommManager_ConnectWiFi(void);

/** 连接上位机 TCP 服务器 */
int CommManager_ConnectServer(const char *ip, uint16_t port);

/** 断开 TCP 连接 */
void CommManager_DisconnectServer(void);

/** 通过 TCP 发送数据到上位机 */
int CommManager_SendToServer(const uint8_t *data, uint16_t len);

/** 获取当前连接状态 */
comm_state_t CommManager_GetState(void);

/** 发送 HMI 消息（printf → USART1） */
void CommManager_SendHMI(uint8_t msg_type, uint8_t severity,
                         const char *text, uint32_t value);

/** 启动 HMI 指令 DMA 接收（循环） */
void CommManager_StartHMIReceive(void);

/** 注册 HMI 指令回调（在 USART DMA 中断上下文中调用） */
void CommManager_RegisterHMICallback(hmi_cmd_cb_t cb);

/**
 * HMI 服务函数：需在 FreeRTOS 任务中周期性调用
 * 负责重启 DMA 接收（不能在 ISR 上下文调 HAL）
 */
void CommManager_ServiceHMI(void);

/**
 * HMI 服务函数：需在 FreeRTOS 任务中周期性调用
 * 负责重启 DMA 接收（不能在 ISR 上下文调 HAL）
 */
void CommManager_ServiceHMI(void);

#endif /* COMM_MANAGER_H */
