# 模块清单与依赖（概要）

说明：下列模块以功能划分，列出职责与主要实现文件，便于快速定位与开发。

- 驱动层（HAL 封装）
  - ADC（模数转换）: 采样传感器、与 DMA 联动
    - 主要文件：[Core/Src/adc.c](Core/Src/adc.c) ；头文件在 [Core/Inc/adc.h](Core/Inc/adc.h)
  - DMA（数据搬运）: 支持 ADC/USART 等外设的高效传输
    - 主要文件：[Core/Src/dma.c](Core/Src/dma.c) ；头文件在 [Core/Inc/dma.h](Core/Inc/dma.h)
  - TIM（定时/触发）: 定时采样与控制周期
    - 主要文件：[Core/Src/tim.c](Core/Src/tim.c) ；头文件在 [Core/Inc/tim.h](Core/Inc/tim.h)
  - USART（串口通讯）: HMI / 外设通信
    - 主要文件：[Core/Src/usart.c](Core/Src/usart.c) ；头文件在 [Core/Inc/usart.h](Core/Inc/usart.h)
  - I2C（外设总线）: 传感器或扩展芯片通信
    - 主要文件：[Core/Src/i2c.c](Core/Src/i2c.c) ；头文件在 [Core/Inc/i2c.h](Core/Inc/i2c.h)
  - GPIO（通用 IO）: 状态指示、控制引脚
    - 主要文件：[Core/Src/gpio.c](Core/Src/gpio.c) ；头文件在 [Core/Inc/gpio.h](Core/Inc/gpio.h)

- 中间件 / RTOS
  - FreeRTOS：任务调度与同步（查看配置文件）
    - 配置： [Core/Inc/FreeRTOSConfig.h](Core/Inc/FreeRTOSConfig.h)
    - 任务定义/管理：项目中 `freertos.c`（[Core/Src/freertos.c](Core/Src/freertos.c)）及 `task_config.h`（[Core/Inc/task_config.h](Core/Inc/task_config.h)）

- 应用层任务（高层职责）
  - 故障保护任务（Fault Protect）
    - 优先级/栈：参见 `task_config.h` 中 `TASK_PRIO_FAULT_PROTECT` 与 `STACK_FAULT_PROTECT`
    - 职责：监控系统健康、强制安全停机或上报
  - 传感采集（Sensor Acquire）
    - 周期：`PERIOD_SENSOR_MS`（见 `task_config.h`）
    - 职责：读取 ADC/I2C 传感器，预处理数据并推送到队列
  - 设备控制（Device Control）
    - 周期/触发：定时或命令驱动
    - 职责：执行控制策略，驱动输出（PWM/GPIO）
  - AI 推理（AI Inference）
    - 预留：`TASK_PRIO_AI_INFERENCE`、`STACK_AI_INFERENCE`
    - 职责：基于采集数据做故障诊断或预测（可选集成 TFLite-Micro）
  - HMI 通信（HMI Comm）
    - 周期：`PERIOD_HMI_REFRESH_MS`
    - 职责：与上位机/显示/触摸交互，发送状态与接收命令

- 启动与系统级
  - 系统初始化（时钟、缓存、外设时钟）: [Core/Src/system_stm32h7xx.c](Core/Src/system_stm32h7xx.c)
  - 中断处理： [Core/Src/stm32h7xx_it.c](Core/Src/stm32h7xx_it.c)
  - HAL MSP 初始化： [Core/Src/stm32h7xx_hal_msp.c](Core/Src/stm32h7xx_hal_msp.c)

- 链接脚本与构建
  - 链接脚本： [STM32H750VBTX_FLASH.ld](STM32H750VBTX_FLASH.ld) 与 [STM32H750VBTX_RAM.ld](STM32H750VBTX_RAM.ld)
  - makefile： [Debug/makefile](Debug/makefile)

开发注意事项与依赖关系
- 驱动层与中断：ADC/DMA/USART 常涉及 ISR 与缓冲区管理，必须保证与 FreeRTOS 的中断优先级约束（配置 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 等）。
- 内存/缓存：H7 系列有独立 D-Cache/I-Cache，DMA 与缓存一致性要注意（在 DMA 前需清/无效缓存）。
- AI 推理：若集成 TFLite-Micro，需要额外评估内存占用（`STACK_AI_INFERENCE` 已预留较大栈），以及是否使用 FPU/CM7 优化指令集。

如果你同意，我可以把 `README.md` 加到仓库（已完成），并基于这个模块清单生成一份任务接口草图（任务队列/消息说明）。