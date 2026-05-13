/* app_tasks.c — FreeRTOS 任务实现（通过 mgr 层操作硬件）*/
#include "app_tasks.h"
#include "task_config.h"
#include "mgr/analog_manager.h"
#include "mgr/sensor_manager.h"
#include "mgr/output_manager.h"
#include "mgr/comm_manager.h"
#include "drv_hal/dma_driver.h"
#include <string.h>
#include <stdio.h>

/* ── RTOS 对象定义 ─────────────────────────────── */
osMessageQueueId_t sensor_queue;
osMessageQueueId_t control_queue;
osMessageQueueId_t ai_result_queue;
osMessageQueueId_t hmi_queue;

osMutexId_t     health_mutex;
osMutexId_t     output_mutex;
osSemaphoreId_t emergency_sem;

health_flags_t   g_health;
task_heartbeat_t g_heartbeat;

/* ── 常量属性 ──────────────────────────────────── */
static const osMessageQueueAttr_t sensor_queue_attr = {
    .name = "sensor_queue"
};
static const osMessageQueueAttr_t control_queue_attr = {
    .name = "control_queue"
};
static const osMessageQueueAttr_t ai_result_queue_attr = {
    .name = "ai_result_queue"
};
static const osMessageQueueAttr_t hmi_queue_attr = {
    .name = "hmi_queue"
};
static const osMutexAttr_t health_mutex_attr = {
    .name = "health_mutex"
};
static const osMutexAttr_t output_mutex_attr = {
    .name = "output_mutex"
};
static const osSemaphoreAttr_t emergency_sem_attr = {
    .name = "emergency_sem"
};

/* ── 初始化函数（由 freertos.c 调用） ──────────── */
void AppTasks_InitRTOSObjects(void)
{
    sensor_queue    = osMessageQueueNew(16, sizeof(sensor_data_t),  &sensor_queue_attr);
    control_queue   = osMessageQueueNew(8,  sizeof(control_cmd_t), &control_queue_attr);
    ai_result_queue = osMessageQueueNew(4,  sizeof(ai_result_t),   &ai_result_queue_attr);
    hmi_queue       = osMessageQueueNew(8,  sizeof(hmi_msg_t),     &hmi_queue_attr);

    health_mutex    = osMutexNew(&health_mutex_attr);
    output_mutex    = osMutexNew(&output_mutex_attr);
    emergency_sem   = osSemaphoreNew(1, 0, &emergency_sem_attr);

    memset((void *)&g_health,    0, sizeof(g_health));
    memset((void *)&g_heartbeat, 0, sizeof(g_heartbeat));
    g_health.adc_ok     = 1;
    g_health.encoder_ok = 1;
    g_health.i2c_ok     = 1;
    g_health.dma_ok     = 1;
    g_health.usart_ok   = 1;
}

/* ═══════════════════════════════════════════════════
 * 故障保护任务 — 最高优先级
 * 周期: 10ms, 监控各任务心跳 + DMA 错误计数 + 紧急停机
 * ═══════════════════════════════════════════════════ */
void StartFaultProtectTask(void *arg)
{
    (void)arg;
    const uint32_t heartbeat_timeout_ms = 500;

    for (;;) {
        g_heartbeat.fault_beat = osKernelGetTickCount();
        uint32_t now = osKernelGetTickCount();

        /* 检查各任务心跳超时 */
        if ((now - g_heartbeat.sensor_beat) > heartbeat_timeout_ms) {
            osMutexAcquire(health_mutex, osWaitForever);
            g_health.task_heartbeat = 0;
            osMutexRelease(health_mutex);
            osSemaphoreRelease(emergency_sem);
            hmi_msg_t alert = { .msg_type = 'E', .severity = 3 };
            snprintf(alert.text, sizeof(alert.text), "Sensor task timeout");
            osMessageQueuePut(hmi_queue, &alert, 0, 0);
        }

        /* 检查 DMA 错误计数（无 mgr 封装，直接读驱动） */
        if (DMA_GetErrorCount() > 10) {
            osMutexAcquire(health_mutex, osWaitForever);
            g_health.dma_ok = 0;
            osMutexRelease(health_mutex);
            osSemaphoreRelease(emergency_sem);
        }

        osDelay(10);
    }
}

/* ── HMI 指令回调（在 ISR 上下文中由 CommManager 调用） ── */
static void on_hmi_cmd(uint8_t cmd_id, int32_t param)
{
    control_cmd_t cmd = { .cmd_id = cmd_id, .param = param };
    osMessageQueuePut(control_queue, &cmd, 0, 0);
}

/* ═══════════════════════════════════════════════════
 * 传感器采集任务 — 高速周期 1ms
 * 模拟量快照 (AnalogManager) + 每 100ms I2C 环境量 (SensorManager)
 * ═══════════════════════════════════════════════════ */
void StartSensorAcquireTask(void *arg)
{
    (void)arg;
    uint8_t  init_done     = 0;
    uint32_t last_i2c_tick = 0;

    for (;;) {
        g_heartbeat.sensor_beat = osKernelGetTickCount();

        /* 首次运行：初始化 mgr 并启动 DMA + 编码器 */
        if (!init_done) {
            AnalogManager_Init();
            AnalogManager_Start();
            SensorManager_Init();
            init_done = 1;
        }

        /* ── 高速快照：模拟量（电流 + 编码器 + ADC 原始值） ── */
        analog_data_t analog;
        uint8_t analog_ok = AnalogManager_ReadAll(&analog);

        if (analog_ok) {
            sensor_data_t data;
            data.timestamp_ms  = analog.timestamp_ms;
            data.adc_raw[0]    = analog.adc_raw[0];
            data.adc_raw[1]    = analog.adc_raw[1];
            data.adc_raw[2]    = analog.adc_raw[2];
            data.encoder_count = analog.encoder_count;
            data.encoder_rpm   = analog.encoder_rpm;
            data.status        = SENSOR_FLAG_ADC_OK | SENSOR_FLAG_ENCODER_OK;

            osMessageQueuePut(sensor_queue, &data, 0, 0);
        }

        /* ── 低速：每 100ms 读一次 I2C 环境传感器 ── */
        uint32_t now = osKernelGetTickCount();
        if ((now - last_i2c_tick) >= PERIOD_HMI_REFRESH_MS) {
            last_i2c_tick = now;

            /* I2C 传感器：SHT30 + LPS22DF + LSM6DSV16X */
            all_sensor_data_t i2c;
            if (SensorManager_ReadAll(&i2c)) {
                hmi_msg_t msg = { .msg_type = 'D', .severity = 0 };
                snprintf(msg.text, sizeof(msg.text),
                         "T:%.1fC H:%.1f%% P:%.1fhPa A:%.2fg G:%.1fdps",
                         i2c.temperature, i2c.humidity, i2c.pressure,
                         i2c.accel_x, i2c.gyro_z);
                osMessageQueuePut(hmi_queue, &msg, 0, 0);
            }

            /* 模拟量摘要上报 HMI */
            if (analog_ok) {
                hmi_msg_t msg2 = { .msg_type = 'D', .severity = 0 };
                snprintf(msg2.text, sizeof(msg2.text),
                         "ADC:%u RPM:%ld ENC:%ld",
                         (unsigned int)(analog.adc_raw[0]),
                         (long)analog.encoder_rpm,
                         (long)analog.encoder_count);
                msg2.value = analog.adc_raw[0];
                osMessageQueuePut(hmi_queue, &msg2, 0, 0);
            }
        }

        osDelay(PERIOD_SENSOR_MS);
    }
}

/* ═══════════════════════════════════════════════════
 * 设备控制任务 — 10ms 周期
 * 处理控制指令, 驱动 PWM, 状态机, LED/Buzzer
 * ═══════════════════════════════════════════════════ */
typedef enum {
    STATE_IDLE      = 0,
    STATE_RUNNING   = 1,
    STATE_FAULT     = 2,
    STATE_ESTOP     = 3
} sys_state_t;

void StartDeviceControlTask(void *arg)
{
    (void)arg;
    uint8_t     init_done = 0;
    sys_state_t state     = STATE_IDLE;
    sensor_data_t sensor;
    control_cmd_t cmd;
    uint8_t pwm_duty = 0;

    for (;;) {
        g_heartbeat.control_beat = osKernelGetTickCount();

        if (!init_done) {
            OutputManager_Init();
            init_done = 1;
        }

        /* 处理控制指令（非阻塞） */
        while (osMessageQueueGet(control_queue, &cmd, NULL, 0) == osOK) {
            switch (cmd.cmd_id) {
            case CMD_START:
                if (state == STATE_IDLE) {
                    state = STATE_RUNNING;
                }
                break;
            case CMD_STOP:
                if (state == STATE_RUNNING) {
                    OutputManager_SetPWM(0);
                    state = STATE_IDLE;
                }
                break;
            case CMD_EMERGENCY_STOP:
                OutputManager_EmergencyStop();
                state = STATE_ESTOP;
                break;
            case CMD_SET_PWM_DUTY:
                if (state == STATE_RUNNING && cmd.param >= 0 && cmd.param <= 100) {
                    pwm_duty = (uint8_t)cmd.param;
                    OutputManager_SetPWM(pwm_duty);
                }
                break;
            default:
                break;
            }
        }

        /* 读取传感器数据 */
        if (osMessageQueueGet(sensor_queue, &sensor, NULL, 0) == osOK) {
            if (!(sensor.status & SENSOR_FLAG_ADC_OK)) {
                osMutexAcquire(health_mutex, osWaitForever);
                g_health.adc_ok = 0;
                osMutexRelease(health_mutex);
            }
        }

        /* 读取 AI 推理结果 */
        ai_result_t ai_res;
        if (osMessageQueueGet(ai_result_queue, &ai_res, NULL, 0) == osOK) {
            if (ai_res.fault_type != FAULT_NONE && ai_res.confidence > 0.8f) {
                state = STATE_FAULT;
                OutputManager_EmergencyStop();
                hmi_msg_t alert = { .msg_type = 'E', .severity = 2 };
                snprintf(alert.text, sizeof(alert.text),
                         "AI fault:%d conf:%.2f", ai_res.fault_type, (double)ai_res.confidence);
                osMessageQueuePut(hmi_queue, &alert, 0, 0);
            }
        }

        /* 状态指示灯 */
        switch (state) {
        case STATE_IDLE:
            OutputManager_SetGreen(LED_ON);
            OutputManager_SetYellow(LED_OFF);
            OutputManager_SetRed(LED_OFF);
            OutputManager_SetBuzzer(0);
            break;
        case STATE_RUNNING:
            OutputManager_SetGreen(LED_OFF);
            OutputManager_SetYellow(LED_ON);
            OutputManager_SetRed(LED_OFF);
            OutputManager_SetBuzzer(0);
            break;
        case STATE_FAULT:
        case STATE_ESTOP:
            OutputManager_SetGreen(LED_OFF);
            OutputManager_SetYellow(LED_OFF);
            OutputManager_SetRed(LED_ON);
            OutputManager_SetBuzzer(1);
            break;
        }

        osDelay(PERIOD_CONTROL_MS);
    }
}

/* ═══════════════════════════════════════════════════
 * AI 推理任务 — 500ms 周期
 * 预留: TFLite-Micro / CMSIS-NN 推理引擎
 * ═══════════════════════════════════════════════════ */
#define AI_MAX_SAMPLES_PER_CYCLE  100

void StartAIInferenceTask(void *arg)
{
    (void)arg;

    for (;;) {
        g_heartbeat.ai_beat = osKernelGetTickCount();

        /* 收集传感器数据窗口（上限 100 条，避免清空队列） */
        sensor_data_t sample;
        uint32_t collected = 0;

        while (collected < AI_MAX_SAMPLES_PER_CYCLE
               && osMessageQueueGet(sensor_queue, &sample, NULL, 0) == osOK) {
            collected++;
        }

        /* 占位：真实推理放在这里 */
        ai_result_t result = {0};
        result.timestamp_ms = HAL_GetTick();
        result.fault_type   = FAULT_NONE;
        result.confidence   = 0.0f;
        result.anomaly_score = 0.0f;

        if (collected > 0) {
            /* TODO: 调用推理模型
             *   例如: TFLite-Micro 推理
             *   result = model_inference(sample_buffer, collected);
             */
            osMessageQueuePut(ai_result_queue, &result, 0, 0);
        }

        osDelay(PERIOD_AI_INFERENCE_MS);
    }
}

/* ═══════════════════════════════════════════════════
 * HMI 通信任务 — 100ms 周期
 * USART1 printf 输出 + 命令帧解析, ESP8266 WiFi/TCP
 * ═══════════════════════════════════════════════════ */
void StartHMICommTask(void *arg)
{
    (void)arg;
    uint8_t init_done = 0;

    for (;;) {
        g_heartbeat.hmi_beat = osKernelGetTickCount();

        if (!init_done) {
            CommManager_Init();
            CommManager_RegisterHMICallback(on_hmi_cmd);

            /* 启动消息 */
            printf("\r\n=== STM32H7 Industrial Diagnostic System ===\r\n");
            printf("System boot OK, Freq=%lu MHz\r\n",
                   (unsigned long)(HAL_RCC_GetSysClockFreq() / 1000000));
            init_done = 1;
        }

        /* 重启 DMA 接收（不能在 ISR 调 HAL，由任务轮询执行） */
        CommManager_ServiceHMI();

        /* 处理出站 HMI 消息 */
        hmi_msg_t msg;
        while (osMessageQueueGet(hmi_queue, &msg, NULL, 0) == osOK) {
            CommManager_SendHMI(msg.msg_type, msg.severity, msg.text, msg.value);
        }

        /* 心跳 */
        printf("heartbeat: S=%lu C=%lu A=%lu H=%lu F=%lu\r\n",
               (unsigned long)g_heartbeat.sensor_beat,
               (unsigned long)g_heartbeat.control_beat,
               (unsigned long)g_heartbeat.ai_beat,
               (unsigned long)g_heartbeat.hmi_beat,
               (unsigned long)g_heartbeat.fault_beat);

        osDelay(PERIOD_HMI_REFRESH_MS);
    }
}
