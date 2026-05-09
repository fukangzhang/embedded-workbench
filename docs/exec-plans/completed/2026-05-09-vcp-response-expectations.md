# VCP Response Expectations

## 目标

增强 FreeRTOS USART2 VCP 检查脚本，让它能对串口 transcript 中的关键响应文本做自动检查，为后续真实板上验证留下更可靠的证据。

## 范围

- 给 `scripts/verify_freertos_usart2_vcp.ps1` 增加默认期望文本检查。
- 支持关闭默认检查或传入自定义期望文本。
- 更新 bring-up 文档和学习文档。

## 不在本次范围

- 不连接真实串口。
- 不改变固件或命令协议。
- 不做复杂协议解析，只做 transcript 关键文本包含检查。

## 验证方式

- `verify_freertos_usart2_vcp.ps1 -DryRun`
- `verify_freertos_usart2_vcp.ps1 -DryRun -NoExpect`
- `verify_freertos_usart2_vcp.ps1 -ListPorts -DryRun`
- `git diff --check`

## 未决问题

- 真正的期望检查仍需要连接真实板卡后执行。

## 进度记录

- 2026-05-09：创建计划，准备增强 VCP 验证脚本的响应判定能力。
- 2026-05-09：`verify_freertos_usart2_vcp.ps1` 增加默认期望片段、`-NoExpect` 和自定义 `-Expect`。
- 2026-05-09：更新 NUCLEO-F401RE bring-up 清单和 VCP 学习文档。
- 2026-05-09：默认 dry-run、`-NoExpect` dry-run、`-ListPorts -DryRun` 和 `git diff --check` 均通过；当前机器无可用串口，未执行真实 VCP 检查。
