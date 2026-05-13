#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "cmsis_os.h"
#include <stdint.h>

/* ── 传感器数据 ────────────────────────────────── */
typedef struct {
    uint32_t timestamp_ms;
    uint32_t adc_raw[3];       /* ADC1 channel 3/4/5 */
    int32_t  encoder_count;
    int32_t  encoder_rpm;
    uint16_t status;           /* 传感器状态标志 */
} sensor_data_t;

/* 传感器状态位 */
#define SENSOR_FLAG_ADC_OK      (1u << 0)
#define SENSOR_FLAG_ENCODER_OK  (1u << 1)
#define SENSOR_FLAG_I2C_OK      (1u << 2)

/* ── 控制指令 ──────────────────────────────────── */
typedef struct {
    uint8_t  cmd_id;
    int32_t  param;
} control_cmd_t;

#define CMD_NONE              0
#define CMD_START             1
#define CMD_STOP              2
#define CMD_EMERGENCY_STOP    3
#define CMD_SET_PWM_DUTY      4
#define CMD_SET_THRESHOLD     5

/* ── AI 推理结果 ───────────────────────────────── */
typedef struct {
    uint32_t timestamp_ms;
    uint8_t  fault_type;       /* 0=正常, 1=过流, 2=过温, 3=振动异常, 4=轴承故障 */
    float    confidence;       /* 置信度 0..1 */
    float    anomaly_score;    /* 异常评分 */
} ai_result_t;

#define FAULT_NONE            0
#define FAULT_OVERCURRENT     1
#define FAULT_OVERTEMP        2
#define FAULT_VIBRATION       3
#define FAULT_BEARING         4

/* ── HMI 消息 ──────────────────────────────────── */
typedef struct {
    uint8_t  msg_type;         /* 'S'=状态, 'D'=数据, 'E'=错误, 'C'=命令回显 */
    uint8_t  severity;         /* 0=info, 1=warn, 2=error, 3=fatal */
    char     text[64];
    uint32_t value;
} hmi_msg_t;

/* ── 系统健康标志（各模块上报） ───────────────── */
typedef struct {
    volatile uint32_t adc_ok       : 1;
    volatile uint32_t encoder_ok   : 1;
    volatile uint32_t i2c_ok       : 1;
    volatile uint32_t dma_ok       : 1;
    volatile uint32_t usart_ok     : 1;
    volatile uint32_t ai_ok        : 1;
    volatile uint32_t task_heartbeat : 1;
    volatile uint32_t reserved     : 25;
} health_flags_t;

/* 任务心跳计数器 */
typedef struct {
    volatile uint32_t sensor_beat;
    volatile uint32_t control_beat;
    volatile uint32_t ai_beat;
    volatile uint32_t hmi_beat;
    volatile uint32_t fault_beat;
} task_heartbeat_t;

/* ── 全局句柄（extern） ────────────────────────── */
extern osMessageQueueId_t sensor_queue;       /* sensor_data_t, 16 deep */
extern osMessageQueueId_t control_queue;      /* control_cmd_t,  8 deep */
extern osMessageQueueId_t ai_result_queue;    /* ai_result_t,    4 deep */
extern osMessageQueueId_t hmi_queue;          /* hmi_msg_t,      8 deep */

extern osMutexId_t       health_mutex;        /* protects health_flags */
extern osMutexId_t       output_mutex;        /* serializes PWM/GPIO output */
extern osSemaphoreId_t   emergency_sem;       /* posted on emergency-stop */

extern health_flags_t    g_health;
extern task_heartbeat_t  g_heartbeat;

/* ── 任务入口声明 ─────────────────────────────── */
void StartFaultProtectTask(void *arg);
void StartSensorAcquireTask(void *arg);
void StartDeviceControlTask(void *arg);
void StartAIInferenceTask(void *arg);
void StartHMICommTask(void *arg);

#endif /* APP_TASKS_H */
