# FreeRTOS Command Ingress Hook

## 目标

在 FreeRTOS port 中新增可选串口命令 ingress hook，让 `communication_task` 可以从 reader callback 有界读取字节，并把解析出的 `command_t` 投递到 `command_queue`。

## 范围

- 在 `freertos_rtos_port_context_t` 中新增 command reader hook。
- 在 `communication_task` 中初始化 `serial_command_ingress`，并周期性调用 `serial_command_ingress_pump_poll`。
- 保持未配置 reader 时现有 `command_queue` 行为不变。
- 补充设计文档、学习文档和 README 索引。

## 不在本次范围

- 不直接绑定真实 USART2 寄存器。
- 不启动真实板上 scheduler 验证。
- 不改变文本命令协议。

## 验证方式

- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`
- 固件构建矩阵：
  - `cmake --build build-fw --clean-first`
  - `cmake --build build-fw-real-usart2-command --clean-first`
  - `cmake --build build-fw-rtos --clean-first`
  - `cmake --build build-fw-scheduler --clean-first`

## 进度记录

- 2026-05-09：从已合并的串口命令 ingress pump 后创建计划，准备把 ingress pump 接入 FreeRTOS communication task。
- 2026-05-09：在 `freertos_rtos_port_context_t` 中新增可选 command reader hook，并在 `communication_task` 中接入 `serial_command_ingress_pump_poll`。
- 2026-05-09：补充学习笔记、设计边界和 README 索引。
- 2026-05-09：主机 37 个测试、普通固件、USART2 命令固件、FreeRTOS 固件和 scheduler 固件构建均通过。
