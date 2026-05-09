# Serial Command Ingress

## 目标

新增串口命令入口模块，把逐字节串口输入组装成命令行并解析为 `command_t`，再通过回调提交给后续队列/任务使用。

## 范围

- 新增 `serial_command_ingress` 头文件和实现。
- 复用 `serial_line` 和 `command_parser`，不重复写行缓冲或解析逻辑。
- 为有效命令和无效命令都提交 `command_t`，让后续响应层统一处理。
- 补充主机测试、学习文档、设计边界和 README 索引。

## 不在本次范围

- 不直接改 FreeRTOS task。
- 不直接接真实 USART2。
- 不改变现有 `serial_command_service` 裸机响应链路。

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

- 2026-05-09：从已合并的 FreeRTOS runtime context 后创建计划，准备补串口字节到 `command_t` 的队列入口。
- 2026-05-09：新增 `serial_command_ingress` 模块，复用 `serial_line` 和 `command_parser`，通过 submit callback 输出 `command_t`。
- 2026-05-09：新增 `tests/test_serial_command_ingress.c` 并接入 CMake，主机测试数量更新为 36 个。
- 2026-05-09：补充学习笔记、设计边界和 README 索引；主机构建、36 个测试和固件构建矩阵均通过。
