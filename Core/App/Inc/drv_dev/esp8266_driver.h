#ifndef ESP8266_DRIVER_H
#define ESP8266_DRIVER_H

#include <stdint.h>
#include <stddef.h>

/* WiFi 连接信息（根据实际环境修改） */
#define ESP_WIFI_SSID       "YOUR_SSID"
#define ESP_WIFI_PASSWORD   "YOUR_PASSWORD"

/* 上位机 TCP 服务器信息 */
#define ESP_SERVER_IP       "192.168.1.100"
#define ESP_SERVER_PORT     8080

/* 接收缓冲区大小 */
#define ESP_RX_BUF_SIZE     512

/* ESP8266 连接状态 */
typedef enum {
    ESP_STATE_IDLE = 0,
    ESP_STATE_READY,
    ESP_STATE_WIFI_CONNECTED,
    ESP_STATE_SERVER_CONNECTED,
    ESP_STATE_ERROR
} esp_state_t;

/**
 * 初始化 ESP8266：检查 AT 响应、设 Station 模式
 * @return 0 成功，非 0 失败
 */
int ESP8266_Init(void);

/**
 * 连接 WiFi 热点
 * @param ssid     WiFi 名称
 * @param password WiFi 密码
 * @return 0 成功
 */
int ESP8266_ConnectWiFi(const char *ssid, const char *password);

/**
 * 连接上位机 TCP 服务器
 * @param ip   服务器 IP 地址
 * @param port 服务器端口
 * @return 0 成功
 */
int ESP8266_ConnectServer(const char *ip, uint16_t port);

/**
 * 通过 TCP 发送数据到上位机
 * @param data 数据指针
 * @param len  数据长度
 * @return 0 成功
 */
int ESP8266_Send(const uint8_t *data, uint16_t len);

/**
 * 检查是否有接收数据可用
 * @return 可读字节数
 */
uint16_t ESP8266_Available(void);

/**
 * 读取接收缓冲区的数据
 * @param buf 输出缓冲区
 * @param len 要读取的长度
 * @return 实际读取的字节数
 */
uint16_t ESP8266_Read(uint8_t *buf, uint16_t len);

/**
 * 获取当前连接状态
 */
esp_state_t ESP8266_GetState(void);

/**
 * 断开 TCP 连接
 */
void ESP8266_Disconnect(void);

#endif
