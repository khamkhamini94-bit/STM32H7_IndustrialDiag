#include "drv_dev/esp8266_driver.h"
#include "drv_hal/usart_driver.h"
#include "stm32h7xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart3;

static volatile esp_state_t state = ESP_STATE_IDLE;


#define ESP_CMD_TO_MS   2000

/* ── 内部：发 AT 命令 ──────────────────────────── */
static int at_send(const char *cmd)
{
    char line[256];
    int len = snprintf(line, sizeof(line), "%s\r\n", cmd);
    return USART3_SendBlocking((uint8_t *)line, (uint32_t)len, ESP_CMD_TO_MS);
}

/* ── 内部：从 USART3 读一个字节（阻塞，短超时） ── */
static int uart_read_byte(uint8_t *ch, uint32_t timeout_ms)
{
    if (HAL_UART_Receive(&huart3, ch, 1, timeout_ms) == HAL_OK)
        return 0;
    return -1;
}

/* ── 内部：读一行到 buffer，返回行长度 ──────────── */
static int uart_read_line(char *line, int maxlen, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    int pos = 0;

    while (pos < maxlen - 1) {
        if ((HAL_GetTick() - start) > timeout_ms) break;

        uint8_t ch;
        if (uart_read_byte(&ch, 10) != 0) continue;  /* 10ms 逐字节轮询 */

        if (ch == '\n') {
            line[pos] = '\0';
            if (pos > 0 && line[pos - 1] == '\r') line[pos - 1] = '\0';
            return pos;
        }
        if (ch != '\r') line[pos++] = (char)ch;
    }
    line[pos] = '\0';
    return (pos > 0) ? pos : -1;
}

/* ── 内部：等待包含指定关键字的行，或超时/出错 ──── */
static int at_wait(const char *keyword, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    char line[128];

    while ((HAL_GetTick() - start) < timeout_ms) {
        int len = uart_read_line(line, sizeof(line),
                   timeout_ms - (HAL_GetTick() - start));
        if (len < 0) continue;
        if (len == 0) continue;

        if (strstr(line, keyword))  return 0;
        if (strstr(line, "ERROR"))  return -1;
        if (strstr(line, "FAIL"))   return -1;
    }
    return -2;  /* 超时 */
}

/* ── 内部：读取 TCP 数据（+IPD 格式）────────────── */
static int at_read_tcp(uint8_t *buf, uint16_t *len, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    char line[128];

    while ((HAL_GetTick() - start) < timeout_ms) {
        int n = uart_read_line(line, sizeof(line),
                timeout_ms - (HAL_GetTick() - start));
        if (n <= 0) continue;

        /* 匹配 +IPD,<len>: 格式 */
        char *ipd = strstr(line, "+IPD,");
        if (ipd) {
            int data_len = atoi(ipd + 5);  /* 跳过 "+IPD," */
            if (data_len <= 0 || data_len > (int)(*len)) continue;

            /* 数据在 ':' 之后，可能跟在同一行或下一行 */
            char *colon = strchr(ipd, ':');
            if (colon) {
                int header_len = (int)(colon + 1 - line);
                int remaining = n - header_len;

                /* 先复制当前行剩余数据 */
                int copied = 0;
                if (remaining > 0) {
                    if (remaining > data_len) remaining = data_len;
                    memcpy(buf, line + header_len, remaining);
                    copied = remaining;
                }

                /* 剩余字节逐字节读取 */
                while (copied < data_len) {
                    uint8_t ch;
                    if (uart_read_byte(&ch, 500) == 0) {
                        buf[copied++] = ch;
                    } else {
                        break;
                    }
                }
                *len = copied;
                return 0;
            }
        }
    }
    return -1;
}

/* ═══════════════════════════════════════════════
 * 公开 API
 * ═══════════════════════════════════════════════ */

int ESP8266_Init(void)
{
    HAL_Delay(500);

    for (int retry = 0; retry < 3; retry++) {
        at_send("AT");
        if (at_wait("OK", 2000) == 0) {
            state = ESP_STATE_READY;
            at_send("AT+CWMODE=1");
            at_wait("OK", 2000);
            /* 关闭回显 */
            at_send("ATE0");
            at_wait("OK", 1000);
            return 0;
        }
        HAL_Delay(500);
    }
    state = ESP_STATE_ERROR;
    return -1;
}

int ESP8266_ConnectWiFi(const char *ssid, const char *password)
{
    if (state < ESP_STATE_READY) return -1;

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);

    at_send(cmd);
    if (at_wait("OK", 15000) != 0) return -1;

    state = ESP_STATE_WIFI_CONNECTED;
    return 0;
}

int ESP8266_ConnectServer(const char *ip, uint16_t port)
{
    if (state < ESP_STATE_WIFI_CONNECTED) return -1;

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%u", ip, port);

    at_send(cmd);
    if (at_wait("OK", 10000) != 0) return -1;
    if (at_wait("CONNECT", 5000) != 0) return -2;

    state = ESP_STATE_SERVER_CONNECTED;
    return 0;
}

int ESP8266_Send(const uint8_t *data, uint16_t len)
{
    if (state != ESP_STATE_SERVER_CONNECTED || data == NULL || len == 0)
        return -1;

    char cmd[20];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%u", len);
    at_send(cmd);

    if (at_wait(">", 5000) != 0) return -2;

    USART3_SendBlocking((uint8_t *)data, len, ESP_CMD_TO_MS);
    HAL_Delay(len / 10 + 50);

    return at_wait("SEND OK", 5000);
}

uint16_t ESP8266_Available(void)
{
    /* 检查 USART3 是否有接收数据 */
    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE))
        return 1;
    return 0;
}

uint16_t ESP8266_Read(uint8_t *buf, uint16_t len)
{
    uint16_t count = 0;
    at_read_tcp(buf, &count, 1000);
    return count;
}

esp_state_t ESP8266_GetState(void)
{
    return state;
}

void ESP8266_Disconnect(void)
{
    at_send("AT+CIPCLOSE");
    at_wait("OK", 3000);
    state = ESP_STATE_WIFI_CONNECTED;
}
