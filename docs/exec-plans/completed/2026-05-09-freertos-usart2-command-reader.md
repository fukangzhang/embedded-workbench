# FreeRTOS USART2 Command Reader

## 目标

给 FreeRTOS 固件路径新增一个默认关闭的真实 USART2 command reader 开关，把 STM32 USART2 单字节 reader 绑定到 `freertos_rtos_port_context_t.command_read`。

## 范围

- 新增 CMake 开关和固件编译宏。
- 在 `firmware/src/main.c` 中初始化真实 USART2 reader，并绑定到 FreeRTOS command ingress hook。
- 使用独立 USART2 serial I/O context，避免被现有模拟自检覆盖。
- 补充 CI 构建、学习文档、设计边界和 README 索引。

## 不在本次范围

- 不把 `response_queue` 写回 USART2。
- 不做真实板上 scheduler 验证。
- 不改变已有裸机 USART2 command loop。

## 验证方式

- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`
- 固件构建矩阵：
  - `cmake --build build-fw --clean-first`
  - `cmake --build build-fw-real-usart2-command --clean-first`
  - `cmake --build build-fw-rtos --clean-first`
  - `cmake --build build-fw-scheduler --clean-first`
  - `cmake --build build-fw-rtos-real-usart2-reader --clean-first`

## 进度记录

- 2026-05-09：从已合并的 FreeRTOS command ingress hook 后创建计划，准备绑定真实 USART2 reader。
- 2026-05-09：新增默认关闭的 `EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER` 开关，并在打开时初始化 STM32F401RE USART2 reader。
- 2026-05-09：补充 CI 固件构建组合、学习文档、设计边界和 README 学习路线。
- 2026-05-09：主机 37 个测试、`git diff --check`、基础固件、真实 USART2 命令 loop、FreeRTOS、scheduler、FreeRTOS 真实 USART2 reader 固件构建均通过。
