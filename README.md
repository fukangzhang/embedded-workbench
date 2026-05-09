# Embedded Workbench

一个面向嵌入式方向求职实践的项目工作台，目标是围绕 `STM32/MCU + FreeRTOS + 通信/传感器/控制`，逐步做出一套可开发、可测试、可讲解、可写进简历的完整项目经历。

## 仓库定位

这个仓库不是单纯放一份代码，而是同时承载：

- 项目代码
- 分阶段开发计划
- 设计说明
- 测试与验证思路
- 求职视角下的项目沉淀

当前阶段优先完成“没有硬件也能推进”的软件骨架、业务逻辑和测试体系，后续再逐步接入真实 `STM32` 目标板环境。

## 目录结构

- `app/`：应用层逻辑、任务、状态机、控制策略
- `bsp/`：板级支持与芯片相关初始化
- `docs/`：设计说明、执行计划、产品说明、参考记录
- `docs/learning/`：跟随项目推进沉淀的嵌入式背景知识和代码讲解
- `drivers/`：驱动与协议适配层
- `firmware/`：固件构建产物或第三方固件资源
- `hardware/`：硬件说明、引脚约定、原理图备注
- `scripts/`：初始化、辅助检查、构建辅助脚本
- `tests/`：主机侧测试与仿真验证
- `tools/`：工具链相关辅助文件

## 当前工作方式

1. 先在 `docs/` 中定义项目目标、阶段计划和设计说明
2. 优先实现可在主机上验证的逻辑模块
3. 通过 `Git` 分支、计划文档和 `PR` 维持清晰的开发过程
4. 后续逐步接入 `FreeRTOS`、通信协议、传感器抽象和控制逻辑

## 当前已完成

到目前为止，仓库已经不是空骨架，已经具备下面这些可展示内容：

- 主机侧 CMake 工程、主机仿真程序 `host_sim`、39 个主机测试和 GitHub CI
- 传感器采样模型、整数样本滤波模块、传感器来源接口、序列模拟传感器来源、滤波传感器来源、采集步骤桥接、环境处理步骤、环境配置安全更新、告警状态机、告警输出策略、闪烁节拍和数字输出抽象
- 文本命令链路：串口行缓冲、串口命令服务、串口命令 ingress、串口命令 ingress pump、串口命令 pump、解析命令、处理配置、SAMPLE 样本注入、格式化响应、命令会话、脚本化 host_sim 输入
- STM32 固件骨架：启动文件、freestanding libc、ELF/BIN/HEX 固件产物、真实 GPIO 初始化开关、USART2 初始化、命令 pump 自检和真实 USART2 命令 loop 开关
- NUCLEO-F401RE bring-up 辅助：OpenOCD dry-run/烧录脚本、FreeRTOS USART2 I/O 固件构建脚本、ST-LINK VCP 验证脚本、ST-LINK/STM32F4 默认配置和板上验证记录入口
- BSP/driver 边界：板级 profile、GPIO 寄存器配置、RCC GPIO/USART 时钟、USART 寄存器 helper、USART 串口 IO 适配器、STM32F401RE GPIO/USART2 地址绑定、板级 USART2 初始化
- FreeRTOS 骨架：任务模型、RTOS port 接口、FreeRTOS task/queue 创建、命令 ingress hook、真实 USART2 command reader/response writer 开关、配置更新队列、scheduler 运行期 context、FreeRTOS demo source 滤波装配、告警事件流和输出节拍接入
- C 代码学习注释：关键 `.c/.h` 文件和测试都已经补充初学者导向注释

## 学习路线

如果你是一边做项目一边学习嵌入式，建议按下面顺序读。不要从所有文档里随机挑，先跟着这条主线建立整体图，再回头补细节。

1. 项目怎么推进：
   - `WORKFLOW.md`
   - `ARCHITECTURE.md`
   - `docs/learning/2026-05-08-项目路线与前后端取舍.md`

2. C 工程和主机验证：
   - `docs/learning/2026-05-08-C工程骨架与主机仿真.md`
   - `docs/learning/2026-05-08-主机测试CI.md`
   - 对应代码：`CMakeLists.txt`、`tools/host_sim/main.c`、`tests/`

3. 业务核心逻辑：
   - `docs/learning/2026-05-08-传感器数据模型.md`
   - `docs/learning/2026-05-09-传感器样本滤波.md`
   - `docs/learning/2026-05-09-传感器来源接口.md`
   - `docs/learning/2026-05-09-序列传感器来源.md`
   - `docs/learning/2026-05-09-滤波传感器来源.md`
   - `docs/learning/2026-05-09-传感器采集步骤.md`
   - `docs/learning/2026-05-09-环境处理步骤.md`
   - `docs/learning/2026-05-08-告警状态机.md`
   - `docs/learning/2026-05-08-告警输出策略.md`
   - `docs/learning/2026-05-08-告警输出节拍逻辑.md`
   - 对应代码：`drivers/include/embedded_workbench/sensor_sample.h`、`drivers/include/embedded_workbench/sensor_sample_filter.h`、`drivers/include/embedded_workbench/sensor_source.h`、`drivers/include/embedded_workbench/sequence_sensor_source.h`、`drivers/include/embedded_workbench/filtered_sensor_source.h`、`app/include/embedded_workbench/sensor_acquisition.h`、`app/include/embedded_workbench/environment_processor.h`、`app/src/alarm_state.c`、`app/src/alarm_output*.c`

4. 串口命令链路：
   - `docs/learning/2026-05-08-串口行缓冲模块.md`
   - `docs/learning/2026-05-09-串口命令服务.md`
   - `docs/learning/2026-05-09-串口命令Ingress.md`
   - `docs/learning/2026-05-09-串口命令IngressPump.md`
   - `docs/learning/2026-05-09-串口命令Pump.md`
   - `docs/learning/2026-05-08-命令解析.md`
   - `docs/learning/2026-05-08-命令处理与配置应用.md`
   - `docs/learning/2026-05-08-命令会话模块.md`
   - `docs/learning/2026-05-09-命令响应步骤.md`
   - `docs/learning/2026-05-09-SAMPLE命令.md`
   - 对应代码：`drivers/src/serial_line.c`、`app/src/serial_command_service.c`、`app/src/serial_command_ingress.c`、`app/src/serial_command_ingress_pump.c`、`app/src/serial_command_pump.c`、`drivers/src/command_parser.c`、`app/src/command_handler.c`、`app/src/command_responder.c`、`app/src/command_session.c`

5. STM32 和硬件边界：
   - `hardware/nucleo-f401re-bringup.md`
   - `docs/learning/2026-05-08-BSP目标板Profile.md`
   - `docs/learning/2026-05-08-STM32GPIO初始化配置.md`
   - `docs/learning/2026-05-08-STM32GPIO输出后端.md`
   - `docs/learning/2026-05-09-STM32GPIOAlternateFunction.md`
   - `docs/learning/2026-05-09-STM32RCCUSART时钟.md`
   - `docs/learning/2026-05-09-STM32USART寄存器Helper.md`
   - `docs/learning/2026-05-09-STM32USART串口IO适配器.md`
   - `docs/learning/2026-05-09-STM32F401REUSART2绑定.md`
   - `docs/learning/2026-05-09-STM32板级USART2初始化.md`
   - `docs/learning/2026-05-09-固件ELF-BIN-HEX产物.md`
   - `docs/learning/2026-05-09-OpenOCD烧录入口.md`
   - `docs/learning/2026-05-09-固件USART2初始化链路自检.md`
   - `docs/learning/2026-05-09-固件USART2命令服务自检.md`
   - `docs/learning/2026-05-09-固件真实USART2命令Loop开关.md`
   - `docs/learning/2026-05-09-串口命令Pump.md`
   - 对应代码：`bsp/`、`drivers/src/stm32_*.c`、`firmware/src/main.c`

6. FreeRTOS 骨架：
   - `docs/learning/2026-05-08-FreeRTOS内核接入预研.md`
   - `docs/learning/2026-05-08-FreeRTOS任务模型.md`
   - `docs/learning/2026-05-08-FreeRTOS队列型RTOSPort骨架.md`
   - `docs/learning/2026-05-08-FreeRTOS告警事件流骨架.md`
   - `docs/learning/2026-05-09-传感器来源接口.md`
   - `docs/learning/2026-05-09-命令响应步骤.md`
   - `docs/learning/2026-05-09-FreeRTOS配置更新队列.md`
   - `docs/learning/2026-05-09-FreeRTOS运行期Context接线.md`
   - `docs/learning/2026-05-09-FreeRTOS命令IngressHook.md`
   - `docs/learning/2026-05-09-FreeRTOS真实USART2命令Reader.md`
   - `docs/learning/2026-05-09-FreeRTOS真实USART2响应Writer.md`
   - `docs/learning/2026-05-09-FreeRTOSSAMPLE命令转发.md`
   - `docs/learning/2026-05-09-FreeRTOS真实USART2IO板上验证入口.md`
   - `docs/learning/2026-05-09-FreeRTOS真实USART2VCP验证脚本.md`
   - 对应代码：`app/src/rtos_task_model.c`、`app/src/rtos_port*.c`、`app/src/serial_command_ingress*.c`、`app/src/command_responder.c`、`firmware/src/main.c`、`firmware/config/FreeRTOSConfig.h`、`scripts/build_freertos_usart2_io_firmware.ps1`、`scripts/verify_freertos_usart2_vcp.ps1`

7. 读 C 代码注释：
   - `docs/learning/2026-05-08-C代码注释阅读方法.md`
   - 这份文档按批次列出“先看哪个文件，再看哪个文件”，适合你对照代码逐段理解。

## 前后端取舍

这个项目目前不急着做 Web 前端。嵌入式项目里的“前端入口”优先是 `host_sim`、串口命令和后续真实板卡串口日志，因为它们更贴近 MCU 工作方式，也更适合面试时讲清楚底层链路。

后续如果为了作品展示和简历增强，可以加一个很薄的 PC dashboard，但它应该消费串口/日志/仿真输出，而不是抢在固件主线前面变成另一个独立 Web 项目。

## 后续可扩展方向

近期优先级建议如下：

1. 真实 NUCLEO-F401RE bring-up：烧录固件、确认 GPIO 输出和失败定位流程
2. UART 接入：把已完成的 USART2 RCC/GPIO AF7/地址绑定和板级初始化接入固件入口，然后把真实串口字节接到 `serial_command_service`
3. 传感器驱动抽象：从 `sensor_source` 接口走向模拟/真实传感器输入
4. FreeRTOS 运行闭环：采集、处理、通信、配置同步、输出任务之间跑通真实队列
5. 协议扩展：在串口命令稳定后，再考虑 `Modbus`、`CAN` 或更完整的 telemetry
6. 展示增强：需要作品集效果时，再增加 PC dashboard 或日志可视化

## 硬件 Bring-up

真实板卡验证前，先看 `hardware/` 下的清单。目前入口是 `hardware/nucleo-f401re-bringup.md`，用于记录 NUCLEO-F401RE 的引脚依据、固件构建、FreeRTOS USART2 I/O 构建脚本、OpenOCD dry-run/烧录入口、ST-LINK VCP 检查脚本、烧录前检查和失败定位顺序。
