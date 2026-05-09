# FreeRTOS USART2 VCP Check

## 目标

为后续真实板上验证准备一个串口 VCP 检查脚本，用固定命令序列验证 FreeRTOS USART2 命令输入和响应输出。

## 范围

- 新增 PowerShell 串口检查脚本，支持 dry-run、端口参数、命令序列和响应日志。
- 更新 NUCLEO-F401RE bring-up 文档和学习索引。

## 不在本次范围

- 不实际连接串口。
- 不声明板上验证通过。
- 不修改固件代码。

## 验证方式

- 脚本 dry-run。
- `git diff --check`

## 未决问题

- 真实验证时需要用户提供 ST-LINK VCP 对应的 `COMx` 端口。

## 进度记录

- 2026-05-09：创建计划，准备补齐串口 VCP 验证入口。
- 2026-05-09：新增 `scripts/verify_freertos_usart2_vcp.ps1`，支持 `-DryRun`、`-ListPorts`、固定命令序列和 transcript 日志。
- 2026-05-09：更新 NUCLEO-F401RE bring-up 清单、学习文档和 README 索引。
- 2026-05-09：脚本 dry-run、串口列表 dry-run 和 `git diff --check` 通过；当前机器未暴露可用串口，未执行真实 VCP 验证。
