# FreeRTOS USART2 I/O Bring-up Entry

## 目标

为真实板上验证准备一个稳定入口：构建 `FreeRTOS scheduler + 真实 USART2 reader + 真实 USART2 writer` 固件，并把对应烧录/串口验证步骤写清楚。

## 范围

- 新增 PowerShell 构建脚本，集中配置本地 Zig/Ninja/FreeRTOS 路径和相关 CMake 开关。
- 在 CI 中增加 scheduler + real USART2 command I/O 构建组合。
- 更新 NUCLEO-F401RE bring-up 文档和学习索引，说明后续板上验证应该用哪个固件。

## 不在本次范围

- 不实际烧录板卡。
- 不声明真实串口已经收发成功。
- 不修改 FreeRTOS 任务逻辑或串口协议。

## 验证方式

- 脚本 dry-run。
- 脚本真实构建。
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

## 未决问题

- 板上验证仍需要真实 NUCLEO-F401RE、OpenOCD 和串口终端。

## 进度记录

- 2026-05-09：创建任务计划，准备补齐 FreeRTOS USART2 I/O scheduler 固件构建入口。
- 2026-05-09：新增 `scripts/build_freertos_usart2_io_firmware.ps1`，默认构建 scheduler + 真实 USART2 reader/writer 固件。
- 2026-05-09：CI 增加 `build-fw-scheduler-real-usart2-io` 构建组合。
- 2026-05-09：更新 NUCLEO-F401RE bring-up 清单、学习文档和 README 索引。
- 2026-05-09：脚本 dry-run、脚本真实构建、OpenOCD 新镜像 dry-run、主机构建、37 个主机测试和 `git diff --check` 均通过。
