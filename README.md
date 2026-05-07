# STM32H7_IndustrialDiag — 项目概览

简介
- 基于 STM32H750VBTx（STM32H7 系列）的工业智能诊断平台。
- 使用 HAL 驱动与 FreeRTOS，多任务架构，并预留 AI 推理任务。

主要文件与入口
- 启动：[Core/Startup/startup_stm32h750vbtx.s](Core/Startup/startup_stm32h750vbtx.s)
- 入口：[Core/Src/main.c](Core/Src/main.c)
- 任务配置：[Core/Inc/task_config.h](Core/Inc/task_config.h)
- 链接脚本：[STM32H750VBTX_FLASH.ld](STM32H750VBTX_FLASH.ld)、[STM32H750VBTX_RAM.ld](STM32H750VBTX_RAM.ld)
- Debug/构建脚本：`Debug/makefile`

环境与先决条件（Windows）
- 安装 GNU Arm Embedded Toolchain (`arm-none-eabi-gcc`, `arm-none-eabi-gdb`) 或通过 `arm-none-eabi` 包获取。
- 安装 Make（例如 MSYS2/MinGW 提供 `make`）。
- 安装 OpenOCD（或使用 STM32CubeProgrammer / ST-Link 工具用于烧写与调试）。
- 推荐 IDE：VSCode + Cortex-Debug，或 STM32CubeIDE（可直接导入工程）。

快速构建
- 在项目根目录使用 Debug 提供的 makefile：

```bash
make -C Debug
```

编译后产物通常在 `Debug/` 目录，名称由 `makefile` 决定（例如 `.elf` / `.bin`）。

烧录（示例）
- 使用 STM32CubeProgrammer (GUI 或 CLI)：

```bash
# 使用 STM32_Programmer_CLI（请根据安装路径调整）
STM32_Programmer_CLI -c port=SWD -w Debug/your_project.elf 0x08000000
```

- 使用 OpenOCD + gdb（示例，替换 target/board 文件为你所用）

```bash
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg \
  -c "program Debug/your_project.elf verify reset exit"
```

调试（示例）
- 使用 OpenOCD 启动调试服务器，然后用 `arm-none-eabi-gdb` 连接：

```bash
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg
arm-none-eabi-gdb Debug/your_project.elf
# 在 gdb 中：
(gdb) target extended-remote :3333
(gdb) monitor reset init
(gdb) load
(gdb) continue
```

- 在 VSCode 推荐安装 `cortex-debug` 扩展并配置 `launch.json` 使用 OpenOCD。

开发流程建议
- 使用分支策略（feature/*）开发新功能，主分支用于稳定交付。
- 在本地搭建能够重复的构建/烧录命令（或脚本），以便 CI 能无差异地执行。
- 编写硬件抽象接口（HAL 封装）与任务间消息/队列契约，便于多人协作。

下一个可交付物（可选）
- 生成更详细的 `CONTRIBUTING.md`（包含开发环境搭建命令、代码风格和 PR 流程）。
- 创建 `scripts/flash.sh` / `scripts/debug.sh` 以简化常用命令。