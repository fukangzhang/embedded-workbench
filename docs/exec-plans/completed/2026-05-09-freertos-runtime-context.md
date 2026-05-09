# FreeRTOS Runtime Context Wiring

## 目标

给固件 FreeRTOS scheduler 路径补上运行期 context 接线：采集任务有默认模拟传感器来源，输出任务有告警输出 sink，并且 scheduler 只在基础自检通过后启动。

## 范围

- 在固件 FreeRTOS 路径中准备 `sequence_sensor_source` 和 `sensor_source_t`。
- 把 `sensor_source` 与 `alarm_output_sink` 写入 `freertos_rtos_port_context_t`。
- 调整 scheduler 启动条件，避免基础自检失败时仍尝试启动调度器。
- 更新学习文档和设计说明。

## 不在本次范围

- 不接真实传感器。
- 不改变 FreeRTOS task 模型。
- 不做真实板上运行验证。

## 验证方式

- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- `cmake --build build-fw --clean-first`
- `cmake --build build-fw-real-usart2-command --clean-first`
- `cmake --build build-fw-rtos --clean-first`
- `cmake --build build-fw-scheduler --clean-first`

## 进度记录

- 2026-05-09：从已合并的配置更新队列后创建计划，准备接 scheduler 运行期资源。
- 2026-05-09：在 `firmware/src/main.c` 中接入模拟序列传感器 source 和告警输出 sink，并把 scheduler 启动保护收紧到基础自检成功后。
- 2026-05-09：补充学习笔记和设计边界说明；主机测试和固件构建矩阵均通过。
