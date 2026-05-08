# NUCLEO-F401RE板级Bringup清单

## Goal

建立 NUCLEO-F401RE 的板级 bring-up 清单，把当前固件真实 GPIO 初始化开关如何在真实板上验证讲清楚。

## Scope

- 新增 `hardware/` 下的板卡说明和 bring-up checklist。
- 记录当前 `board_profile` 使用的告警输出引脚与官方资料依据。
- 明确下载前检查、构建命令、烧录后观察项、失败定位路径。
- 只写文档，不改代码和构建逻辑。

## Steps

1. 根据当前 `board_profile` 和 STM32 Nucleo-64 官方用户手册整理引脚事实。
2. 编写 NUCLEO-F401RE bring-up checklist。
3. 更新学习索引，让你知道这份清单应该在什么时候读。
4. 运行文档改动对应检查并提交。

## Validation

- `git diff --check`
- `git status --short --branch`

## Open Questions

- 当前外部蜂鸣器 `PB6`、执行器 `PB7` 仍是项目占位引脚；真正接线后需要把实物接法记录回来。
- 板上烧录工具和实际观察结果还未执行，本轮只准备标准流程。

## Progress Log

- 2026-05-08：创建计划，开始整理 NUCLEO-F401RE 板级 bring-up 清单。
- 2026-05-08：新增 `hardware/README.md` 和 `hardware/nucleo-f401re-bringup.md`，记录当前引脚映射、官方资料依据、构建命令、烧录前检查和失败定位顺序。
- 2026-05-08：更新 `README.md` 和 `docs/learning/README.md`，把硬件 bring-up 清单纳入阅读路径。
- 2026-05-08：验证通过：`git diff --cached --check`、`git status --short --branch`。
