#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* 任务优先级（数值越大优先级越高）*/
#define TASK_PRIO_FAULT_PROTECT     6
#define TASK_PRIO_SENSOR_ACQUIRE    5
#define TASK_PRIO_DEVICE_CONTROL    4
#define TASK_PRIO_AI_INFERENCE      2
#define TASK_PRIO_HMI_COMM          1

/* 任务栈大小（单位：words）*/
#define STACK_FAULT_PROTECT         256
#define STACK_SENSOR_ACQUIRE        512
#define STACK_DEVICE_CONTROL        512
#define STACK_AI_INFERENCE          2048
#define STACK_HMI_COMM              512

/* 任务周期（ms）*/
#define PERIOD_SENSOR_MS            1
#define PERIOD_AI_INFERENCE_MS      500
#define PERIOD_HMI_REFRESH_MS       100
#define PERIOD_CONTROL_MS           10

