# FreeRTOS SAMPLE Command Forwarding

## 目标

让 FreeRTOS `communication_task` 在处理 `SAMPLE` 命令后，把更新后的样本投递到 `sensor_sample_queue`，让环境处理任务和告警输出链路也能看到手工注入的样本。

## 范围

- 修改 `app/src/rtos_port_freertos.c` 的命令处理逻辑。
- 更新学习文档，说明 `SAMPLE` 在 FreeRTOS 路径里不只更新响应上下文，也会进入环境处理队列。
- 保持命令协议、队列模型和固件开关不变。

## 不在本次范围

- 不新增队列。
- 不改串口命令格式。
- 不做真实板上 VCP 验证。

## 验证方式

- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`
- `cmake --build build-fw-rtos --clean-first`
- `cmake --build build-fw-scheduler-real-usart2-io --clean-first`

## 未决问题

- FreeRTOS 任务级行为仍主要通过固件构建验证，真实调度效果需要后续板上 VCP transcript 记录。

## 进度记录

- 2026-05-09：创建计划，准备把 `SAMPLE` 命令转发到 FreeRTOS sample 队列。
- 2026-05-09：`communication_task` 在 `status.sample_changed` 时调用 `freertos_submit_sensor_sample`，把 responder 的最新样本送入 `sensor_sample_queue`。
- 2026-05-09：补充设计边界、学习文档和 README/学习索引。
- 2026-05-09：`cmake --build build`、37 个主机测试、`git diff --check`、`build-fw-rtos` 和 `build-fw-scheduler-real-usart2-io` 构建均通过。
