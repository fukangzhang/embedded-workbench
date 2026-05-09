# Serial Command Ingress Pump

## 目标

新增串口命令 ingress pump，从 reader callback 有界读取字节并喂给 `serial_command_ingress`，为后续 USART2/FreeRTOS command queue 接入准备轮询外壳。

## 范围

- 新增 `serial_command_ingress_pump` 头文件和实现。
- 复用 `serial_command_ingress_feed`，不重复处理行缓冲和命令解析。
- 补充主机测试、学习文档、设计说明和索引。

## 不在本次范围

- 不直接接 STM32 USART2。
- 不改 FreeRTOS task。
- 不改现有 `serial_command_pump`。

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

- 2026-05-09：从已合并的串口命令 ingress 后创建计划，准备补 ingress pump。
- 2026-05-09：新增 `serial_command_ingress_pump`，用 reader callback 有界读取字节并喂给 ingress。
- 2026-05-09：新增 `tests/test_serial_command_ingress_pump.c` 并接入 CMake，主机测试数量更新为 37 个。
- 2026-05-09：补充学习笔记、设计边界和 README 索引；主机构建、37 个测试和固件构建矩阵均通过。
