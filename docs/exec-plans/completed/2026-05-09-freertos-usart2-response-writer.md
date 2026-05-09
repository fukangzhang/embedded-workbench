# FreeRTOS USART2 Response Writer

## 目标

给 FreeRTOS 固件路径新增一个默认关闭的真实 USART2 response writer 开关，把 `response_queue` 中的响应文本写回 STM32F401RE USART2。

## 范围

- 在 FreeRTOS port context 中增加可选 response writer hook。
- 在 `communication_task` 生成响应后，通过 writer hook 有界写回响应文本。
- 新增 CMake 开关和固件编译宏。
- 在 `firmware/src/main.c` 中复用真实 USART2 serial I/O context，绑定 writer hook。
- 补充 CI 构建、学习文档、设计边界和 README 索引。

## 不在本次范围

- 不修改串口命令协议。
- 不改成中断或 DMA 串口。
- 不做真实板上 scheduler 串口收发验证。

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
  - `cmake --build build-fw-rtos-real-usart2-io --clean-first`

## 进度记录

- 2026-05-09：从已合并的 FreeRTOS USART2 reader 后创建计划，准备补齐 response writer。
- 2026-05-09：新增 `response_write` hook，并在 `communication_task` 中通过 `response_queue -> writer` 写出响应。
- 2026-05-09：新增默认关闭的 `EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_RESPONSE_WRITER` 开关，固件可把 USART2 同时绑定为 reader 和 writer。
- 2026-05-09：补充 CI reader+writer 构建组合、学习文档、设计边界和 README 学习路线。
- 2026-05-09：主机 37 个测试、`git diff --check`、基础固件、真实 USART2 命令 loop、FreeRTOS、scheduler、reader-only、reader+writer 固件构建均通过。
