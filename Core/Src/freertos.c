/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_tasks.h"
#include "task_config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Application task handles */
osThreadId_t faultTaskHandle;
osThreadId_t sensorTaskHandle;
osThreadId_t controlTaskHandle;
osThreadId_t aiTaskHandle;
osThreadId_t hmiTaskHandle;

/* Application task attributes — mapped from task_config.h */
const osThreadAttr_t faultTask_attributes = {
  .name       = "faultTask",
  .stack_size = STACK_FAULT_PROTECT * 4,
  .priority   = (osPriority_t) TASK_PRIO_FAULT_PROTECT,
};

const osThreadAttr_t sensorTask_attributes = {
  .name       = "sensorTask",
  .stack_size = STACK_SENSOR_ACQUIRE * 4,
  .priority   = (osPriority_t) TASK_PRIO_SENSOR_ACQUIRE,
};

const osThreadAttr_t controlTask_attributes = {
  .name       = "controlTask",
  .stack_size = STACK_DEVICE_CONTROL * 4,
  .priority   = (osPriority_t) TASK_PRIO_DEVICE_CONTROL,
};

const osThreadAttr_t aiTask_attributes = {
  .name       = "aiTask",
  .stack_size = STACK_AI_INFERENCE * 4,
  .priority   = (osPriority_t) TASK_PRIO_AI_INFERENCE,
};

const osThreadAttr_t hmiTask_attributes = {
  .name       = "hmiTask",
  .stack_size = STACK_HMI_COMM * 4,
  .priority   = (osPriority_t) TASK_PRIO_HMI_COMM,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern void AppTasks_InitRTOSObjects(void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* 初始化任务间通信对象 */
  AppTasks_InitRTOSObjects();

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* Application tasks — priorities from task_config.h */
  faultTaskHandle   = osThreadNew(StartFaultProtectTask,   NULL, &faultTask_attributes);
  sensorTaskHandle  = osThreadNew(StartSensorAcquireTask,  NULL, &sensorTask_attributes);
  controlTaskHandle = osThreadNew(StartDeviceControlTask,  NULL, &controlTask_attributes);
  aiTaskHandle      = osThreadNew(StartAIInferenceTask,    NULL, &aiTask_attributes);
  hmiTaskHandle     = osThreadNew(StartHMICommTask,        NULL, &hmiTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
