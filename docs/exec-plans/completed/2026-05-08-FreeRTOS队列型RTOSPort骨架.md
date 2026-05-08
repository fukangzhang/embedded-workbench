# 目标
建立第一版 FreeRTOS 队列型 `rtos_port` 实现骨架，让现有应用侧 RTOS 适配接口开始落到真实 FreeRTOS queue API 上。

## 范围
- 新增 FreeRTOS 专用 `rtos_port` 实现
- 使用 `xQueueCreate` 创建传感器、命令、响应三个队列
- 使用 `xQueueSend` / `xQueueReceive` 实现现有 port 操作
- 固件 FreeRTOS 构建下做最小初始化和队列收发自检
- 补充学习笔记

## 暂不做
- 不创建真实任务
- 不调用 `vTaskStartScheduler`
- 不处理 ISR 版本 API
- 不做板上运行验证
- 不改变 host fake port 测试模型

## 步骤
1. 新增 `rtos_port_freertos` 头文件和源文件
2. 在 CMake 中仅当 `EW_USE_FREERTOS=ON` 时编译该实现
3. 在 firmware 自检中初始化 FreeRTOS port 并验证基础队列收发
4. 更新学习文档
5. 运行 host 测试、普通 firmware 构建、FreeRTOS firmware 构建

## 验证方式
- 主机：`cmake --build build --target test`
- 固件无 RTOS：`cmake --build build-fw`
- 固件带 RTOS：`cmake --build build-fw-rtos --clean-first`
- GitHub Actions：主机、固件、FreeRTOS 固件 job 通过

## 未决问题
- `rtos_port_start()` 在真实 FreeRTOS 中最终应调用 `vTaskStartScheduler()`，但该函数正常情况下不返回，后续需要单独调整生命周期语义
- 后续真实任务接入后，需要把 queue handle 传给任务函数或集中放在运行时上下文中

## 进度记录
- 2026-05-08：创建 FreeRTOS 队列型 `rtos_port` 骨架计划
- 2026-05-08：新增 `rtos_port_freertos` 头文件和源文件
- 2026-05-08：使用 `xQueueCreate` 建立传感器、命令、响应三个队列
- 2026-05-08：将 `rtos_port` 发送/接收操作映射到 `xQueueSend` / `xQueueReceive`
- 2026-05-08：固件 FreeRTOS 路径新增最小队列收发自检
- 2026-05-08：补充 `__aeabi_memclr` / `__aeabi_memclr4` / `__aeabi_memclr8` 裸机运行时符号
- 2026-05-08：补充 FreeRTOS 队列型 RTOS port 学习笔记

## 验证记录
- 主机 `cmake -S . -B build -G Ninja ...`：通过
- 主机 `cmake --build build`：通过
- 主机 `cmake --build build --target test`：通过，`10/10` 测试通过
- 固件 `cmake -S . -B build-fw -G Ninja ... -DEW_BUILD_FIRMWARE=ON`：通过
- 固件 `cmake --build build-fw`：通过，生成 `embedded_firmware.elf`
- FreeRTOS 固件 `cmake -S . -B build-fw-rtos ... -DEW_USE_FREERTOS=ON`：通过
- FreeRTOS 固件 `cmake --build build-fw-rtos --clean-first`：通过，生成 `embedded_firmware.elf`

## 完成结果
- 已完成第一版 FreeRTOS queue-backed `rtos_port` 骨架
- 已保留 host fake port 测试路径
- 已为后续真实任务创建和调度器启动打好基础
