# 2026-05-09 FreeRTOS demo source 接入滤波层

## 目标

在 FreeRTOS 固件运行期 context 装配处使用 `filtered_sensor_source` 包装现有模拟传感器来源，让 scheduler demo 路径开始体现“采样来源可组合”的设计。

## 范围

- 修改 `firmware/src/main.c` 的 FreeRTOS demo source 装配。
- 修正滤波实现里会影响 freestanding ARM 链接的 64 位除法依赖。
- 保留当前默认行为边界：仍然是模拟样本，不接真实传感器。
- 更新相关学习笔记和设计说明。
- 不修改 `app/src/rtos_port_freertos.c` 的任务逻辑。

## 步骤

1. 创建分支并确认 `main` 干净。
2. 在固件 FreeRTOS context 初始化中加入 `filtered_sensor_source`。
3. 调整 demo 样本，让滤波后仍能进入 alarm，避免演示链路变弱。
4. 更新学习文档和设计边界。
5. 运行主机测试与固件构建验证。

## 验证方式

- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`
- `cmake --build build-fw --clean-first`
- `cmake --build build-fw-scheduler-real-usart2-io --clean-first`

## 未决问题

- 真实传感器接入后，滤波参数应该进入配置链路还是保持编译期默认，仍需单独设计。

## 进度记录

- 2026-05-09：创建计划，开始接线。
- 2026-05-09：FreeRTOS demo source 已改为 `sequence_sensor_source -> filtered_sensor_source -> context.sensor_source`。
- 2026-05-09：修正 `sensor_sample_filter` 的 64 位除法依赖，避免 FreeRTOS freestanding 固件链接失败。
- 2026-05-09：`cmake --build build`、`ctest --test-dir build --output-on-failure`、`git diff --check`、`cmake --build build-fw --clean-first`、`cmake --build build-fw-scheduler-real-usart2-io --clean-first` 均通过。
