# AGENTS.md

## 项目概况
- 项目名：Embedded Workbench
- 项目目标：围绕 `STM32/MCU + FreeRTOS + 通信/传感器/控制`，逐步做出一个适合求职展示的完整嵌入式项目
- 当前阶段：项目定义与骨架搭建
- 主要语言：`C`
- 构建方式：`CMake`

## 架构入口
- 详细架构规则见 `ARCHITECTURE.md`
- 当前推荐依赖方向：`bsp -> drivers -> app`
- 测试与仿真代码放在 `tests/`
- 计划、设计说明、产品说明放在 `docs/`

## 常用命令
- `cmake -S . -B build`：生成构建目录
- `cmake --build build`：执行构建
- `ctest --test-dir build --output-on-failure`：运行测试
- `git status --short --branch`：查看当前分支与工作区状态

## 项目约定
- 一次只推进一个明确任务，不把功能、重构、文档混在同一次改动里
- 能在主机上验证的纯逻辑模块，优先采用测试驱动或先写验证用例
- 硬件相关代码尽量通过小接口隔离，保证后续可以做仿真和替换
- 新功能开始前，需要先在 `docs/exec-plans/active/` 写简短计划
- 重要设计决策记录到 `docs/design-docs/`
- 参考开源项目时，可以学习结构和思路，但不要直接复制成自己的核心成果

## Git 规则
- `main` 只保留稳定内容，不直接在上面做功能开发
- 分支命名：
  - `feat/<主题>`
  - `fix/<主题>`
  - `docs/<主题>`
  - `chore/<主题>`
  - `test/<主题>`
- 提交信息前缀：
  - `feat:`
  - `fix:`
  - `docs:`
  - `chore:`
  - `test:`

## 每次开始工作前
1. 先看 `AGENTS.md`、`WORKFLOW.md` 和当前活动计划
2. 先执行 `git status`，确认工作区干净程度
3. 明确这次只做哪一个小任务
4. 完成后执行对应验证
5. 更新计划和相关文档，再提交代码

## 知识索引
- 工作流程：`WORKFLOW.md`
- 架构规则：`ARCHITECTURE.md`
- 当前计划：`docs/exec-plans/active/`
- 完成计划：`docs/exec-plans/completed/`
- 设计文档：`docs/design-docs/`
- 产品说明：`docs/product-specs/`
- 学习笔记：`docs/learning/`
