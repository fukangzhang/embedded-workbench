# VCP Transcript Replay

## 目标

给 FreeRTOS USART2 VCP 验证脚本增加离线 transcript 回放检查能力，让期望响应匹配逻辑可以在没有真实板卡时验证，也方便后续复查保存下来的串口日志。

## 范围

- `scripts/verify_freertos_usart2_vcp.ps1` 增加 `-ReplayLogPath`。
- 回放模式读取已有 transcript，并复用 `-Expect` / `-NoExpect` 逻辑。
- 更新 bring-up 文档和学习文档。

## 不在本次范围

- 不连接真实串口。
- 不修改固件代码。
- 不做复杂协议解析，只检查关键文本片段。

## 验证方式

- 脚本 dry-run。
- 构造通过 transcript，回放应通过。
- 构造缺失期望 transcript，回放应失败。
- `git diff --check`

## 未决问题

- 真实 transcript 仍要等 NUCLEO-F401RE 接入后生成。

## 进度记录

- 2026-05-09：创建计划，准备补离线 transcript 回放检查。
- 2026-05-09：`verify_freertos_usart2_vcp.ps1` 增加 `-ReplayLogPath`，复用默认/自定义/关闭期望检查。
- 2026-05-09：更新 VCP 学习文档、bring-up 清单和学习索引。
- 2026-05-09：dry-run、通过 transcript 回放、缺失期望 transcript 失败验证、`-NoExpect` 回放和 `git diff --check` 均通过。
