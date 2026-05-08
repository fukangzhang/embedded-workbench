# 学习笔记入口

这个目录不是教材仓库，而是 Embedded Workbench 的“代码导读”。每篇学习笔记都应该能回答三个问题：

- 这一步为什么要做？
- 它对应仓库里的哪些代码？
- 看完以后，你应该能用自己的话解释什么？

建议你不要按文件名随便点开看，而是按下面路线读。

## 先看哪里？

如果你今天刚打开项目，只看这 4 个入口：

1. `../product-specs/项目说明.md`
2. `../../ARCHITECTURE.md`
3. `../../WORKFLOW.md`
4. `2026-05-08-项目路线与前后端取舍.md`

看完以后再回到代码目录，只认识这几个顶层目录就够：

- `app/`：业务逻辑，比如告警、响应、RTOS 任务模型
- `drivers/`：可复用驱动和协议解析，比如传感器数据、命令解析
- `bsp/`：板级信息，比如目标板 profile
- `firmware/`：目标板固件入口、启动文件、链接脚本、FreeRTOS 配置
- `tests/`：主机侧测试
- `tools/host_sim/`：主机仿真程序
- `docs/`：计划、设计、学习笔记

## 推荐阅读顺序

### 0. 项目路线

先看：

- `2026-05-08-项目路线与前后端取舍.md`
- `2026-05-08-C工程骨架与主机仿真.md`
- `2026-05-08-Windows-C工具链准备.md`

对应代码：

- `CMakeLists.txt`
- `app/include/embedded_workbench/app_info.h`
- `app/src/app_info.c`
- `tools/host_sim/main.c`
- `tests/test_app_info.c`

看完要能解释：

- 为什么这个项目先做 C 主机仿真，而不是一上来烧板子
- CMake 在这里负责什么
- `host_sim` 和 `tests/` 的区别是什么

### 1. 传感器与告警核心逻辑

先看：

- `2026-05-08-传感器数据模型.md`
- `2026-05-08-告警状态机.md`
- `2026-05-08-告警输出策略.md`
- `2026-05-08-告警输出节拍逻辑.md`
- `2026-05-08-主机仿真告警输出状态展示.md`
- `2026-05-08-告警输出Sink接口.md`
- `2026-05-08-告警输出数字输出适配器.md`

对应代码：

- `drivers/include/embedded_workbench/sensor_sample.h`
- `drivers/src/sensor_sample.c`
- `app/include/embedded_workbench/alarm_state.h`
- `app/src/alarm_state.c`
- `app/include/embedded_workbench/alarm_output.h`
- `app/src/alarm_output.c`
- `app/include/embedded_workbench/alarm_output_timing.h`
- `app/src/alarm_output_timing.c`
- `app/include/embedded_workbench/alarm_output_sink.h`
- `app/src/alarm_output_sink.c`
- `app/include/embedded_workbench/alarm_output_digital_sink.h`
- `app/src/alarm_output_digital_sink.c`
- `app/src/response_format.c`
- `tools/host_sim/main.c`
- `tests/test_sensor_sample.c`
- `tests/test_alarm_state.c`
- `tests/test_alarm_output.c`
- `tests/test_alarm_output_timing.c`
- `tests/test_alarm_output_sink.c`
- `tests/test_alarm_output_digital_sink.c`
- `tests/test_response_format.c`

看完要能解释：

- 一个传感器样本里有哪些字段
- 为什么要校验传感器数据范围
- warning/alarm/recovery 这些阈值怎么影响状态切换
- 为什么“告警状态”和“硬件输出策略”要分成两个模块
- `period_ms` 如何变成 indicator 的亮灭节拍
- 主机仿真里看到的 output 字段来自哪个模块
- `sink` 接口为什么能把业务逻辑和真实 GPIO 隔离开
- 适配器如何把告警输出映射到 `board_profile` 的三类输出脚

### 2. 命令、配置和响应

先看：

- `2026-05-08-命令解析.md`
- `2026-05-08-命令处理与配置应用.md`
- `2026-05-08-响应格式规范.md`
- `2026-05-08-主机仿真交互闭环.md`

对应代码：

- `drivers/include/embedded_workbench/command_parser.h`
- `drivers/src/command_parser.c`
- `app/include/embedded_workbench/command_handler.h`
- `app/src/command_handler.c`
- `app/include/embedded_workbench/response_format.h`
- `app/src/response_format.c`
- `tools/host_sim/main.c`
- `tests/test_command_parser.c`
- `tests/test_command_handler.c`
- `tests/test_response_format.c`

看完要能解释：

- 文本命令怎么被解析成结构体
- 配置命令为什么不直接散落在 main 函数里处理
- 响应格式为什么要单独模块化
- 主机仿真如何串起“输入命令 -> 处理 -> 输出响应”

### 3. CI 和可回溯开发流程

先看：

- `2026-05-08-主机测试CI.md`
- `2026-05-08-固件构建CI.md`

对应代码：

- `.github/workflows/ci.yml`
- `tests/`
- `cmake/toolchains/zig-arm-none-eabi.cmake`

看完要能解释：

- GitHub Actions 当前跑了哪些检查
- host tests 验证什么
- firmware ELF build 验证什么
- 为什么 CI 通过不等于板上已经运行成功

### 4. BSP、固件骨架和目标板构建

先看：

- `2026-05-08-BSP目标板Profile.md`
- `2026-05-08-数字输出驱动抽象.md`
- `2026-05-08-板级数字输出后端.md`
- `2026-05-08-STM32GPIO输出后端.md`
- `2026-05-08-STM32GPIO初始化配置.md`
- `2026-05-08-目标板工具链预研.md`
- `2026-05-08-STM32固件工程骨架.md`
- `2026-05-08-固件告警输出链路自检.md`

对应代码：

- `bsp/include/embedded_workbench/board_profile.h`
- `bsp/src/board_profile.c`
- `drivers/include/embedded_workbench/digital_output.h`
- `drivers/src/digital_output.c`
- `drivers/include/embedded_workbench/board_digital_output.h`
- `drivers/src/board_digital_output.c`
- `drivers/include/embedded_workbench/stm32_gpio_output.h`
- `drivers/src/stm32_gpio_output.c`
- `drivers/include/embedded_workbench/stm32_gpio_config.h`
- `drivers/src/stm32_gpio_config.c`
- `firmware/src/main.c`
- `firmware/startup/startup_stm32f401re.c`
- `firmware/linker/stm32f401re.ld`
- `cmake/toolchains/arm-none-eabi.cmake`
- `cmake/toolchains/zig-arm-none-eabi.cmake`
- `tests/test_board_profile.c`
- `tests/test_digital_output.c`
- `tests/test_board_digital_output.c`
- `tests/test_stm32_gpio_output.c`
- `tests/test_stm32_gpio_config.c`

看完要能解释：

- BSP 和 app 的边界在哪里
- `alarm_led`、`alarm_buzzer`、`alarm_actuator` 只是板级映射，不是 GPIO 驱动
- `digital_output` 为什么属于驱动抽象，不应该知道告警状态机
- `board_digital_output` 为什么只是 profile-backed 后端，还不是 STM32 GPIO 实现
- `stm32_gpio_output` 如何把 high/low 转成 STM32 `BSRR` 写入
- `stm32_gpio_config` 如何配置 `MODER/OTYPER/OSPEEDR/PUPDR`
- `firmware/src/main.c` 现在做了哪些自检
- 固件自检为什么能覆盖输出链路链接，但不能证明真实 GPIO 已经动作
- startup 文件和 linker script 是什么角色
- 为什么当前 Zig 路径是构建验证，不是最终板上工具链

### 5. RTOS 抽象层

先看：

- `2026-05-08-FreeRTOS任务模型.md`
- `2026-05-08-RTOS适配接口.md`

对应代码：

- `app/include/embedded_workbench/rtos_task_model.h`
- `app/src/rtos_task_model.c`
- `app/include/embedded_workbench/rtos_port.h`
- `app/src/rtos_port.c`
- `tests/test_rtos_task_model.c`
- `tests/test_rtos_port.c`

看完要能解释：

- 项目里规划了哪些 FreeRTOS task 和 queue
- 为什么应用层不应该到处直接调用 FreeRTOS API
- `rtos_port` 操作表像 C 语言里的接口
- host fake 测试为什么能在没有 RTOS 的情况下验证接口行为

### 6. FreeRTOS 接入路径

按顺序看：

- `2026-05-08-FreeRTOS内核接入预研.md`
- `2026-05-08-FreeRTOS队列型RTOSPort骨架.md`
- `2026-05-08-FreeRTOS任务创建骨架.md`
- `2026-05-08-FreeRTOS调度器显式启动入口.md`
- `2026-05-08-FreeRTOS告警事件流骨架.md`
- `2026-05-08-FreeRTOS告警输出节拍接入.md`

对应代码：

- `cmake/FreeRTOSKernel.cmake`
- `firmware/config/FreeRTOSConfig.h`
- `firmware/libc/include/stdlib.h`
- `firmware/libc/include/string.h`
- `firmware/libc/src/memory.c`
- `app/include/embedded_workbench/rtos_port_freertos.h`
- `app/src/rtos_port_freertos.c`
- `firmware/src/main.c`

看完要能解释：

- 为什么不把 FreeRTOS-Kernel 源码直接复制进仓库
- `FreeRTOSConfig.h` 是项目侧配置，不是内核自带固定答案
- queue 如何连接 task 之间的数据流
- `xTaskCreate` 创建任务，但不等于调度器已经启动
- `vTaskStartScheduler` 为什么要通过显式开关控制
- 告警输出 task 为什么是“事件更新状态 + 周期刷新 indicator”
- `period_ms` 如何通过 `alarm_output_timing` 接入 FreeRTOS 输出链路
- 当前 FreeRTOS 验证是构建和链接级，板上运行验证还没做

## 代码阅读建议

每一阶段都按这个顺序看：

1. 先看对应学习笔记，只抓“为什么”
2. 再看头文件 `*.h`，理解模块对外暴露什么
3. 再看源文件 `*.c`，理解内部怎么实现
4. 最后看 `tests/test_*.c`，确认哪些行为已经被测试固定下来

不要从 `firmware/src/main.c` 开始硬啃。现在的 `main.c` 是集成入口，它会引用很多模块；先看模块，再看入口会顺很多。

## 当前代码主线怎么串起来？

可以用这条线理解：

```text
sensor_sample
  -> alarm_state
  -> alarm_output
  -> alarm_output_timing
  -> alarm_output_sink
  -> alarm_output_digital_sink
  -> digital_output
  -> board_digital_output

command_parser
  -> command_handler
  -> response_format

rtos_task_model
  -> rtos_port
  -> rtos_port_freertos
  -> alarm_output_sink
  -> firmware/main.c
```

主机侧验证路径：

```text
tests/test_*.c
  -> app/ + drivers/
  -> ctest
```

固件侧验证路径：

```text
firmware/src/main.c
  -> app/ + drivers/ + bsp/
  -> embedded_firmware.elf
```

FreeRTOS 构建路径：

```text
firmware/src/main.c
  -> rtos_port_freertos
  -> FreeRTOS-Kernel
  -> embedded_firmware.elf
```

## 如果你只有 30 分钟

只看这些：

1. `2026-05-08-项目路线与前后端取舍.md`
2. `2026-05-08-传感器数据模型.md`
3. `2026-05-08-告警状态机.md`
4. `2026-05-08-FreeRTOS任务模型.md`
5. `2026-05-08-FreeRTOS队列型RTOSPort骨架.md`

然后打开这些代码：

1. `app/src/alarm_state.c`
2. `app/src/rtos_task_model.c`
3. `app/src/rtos_port_freertos.c`
4. `tests/test_alarm_state.c`

这会让你先抓住项目最核心的“传感器 -> 告警 -> RTOS 任务协作”主线。

## 笔记规则

- 每篇只解释当前阶段真正用到的概念
- 尽量配合本项目代码举例
- 记录“为什么这样做”，不只记录“怎么写”
- 可以补充面试表达，但不把简历包装放在代码之前
