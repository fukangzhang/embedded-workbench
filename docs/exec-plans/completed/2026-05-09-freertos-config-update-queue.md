# FreeRTOS Config Update Queue

## 目标

新增 FreeRTOS 内部配置更新队列，让通信任务在 `SET` 命令成功后可以把新的 `alarm_config_t` 快照同步给环境处理任务。

## 范围

- 在 RTOS task model 中新增 `config_update_queue`。
- 在 FreeRTOS context 中创建并保存配置更新队列。
- 扩展 `command_responder` 输出元数据，标记配置是否发生变化。
- 通信任务在配置变化后发送配置快照。
- 环境处理任务接收配置快照并调用 `environment_processor_update_config`。
- 补充测试、设计文档和学习文档。

## 不在本次范围

- 不改变文本命令协议。
- 不做真实板上调度器运行验证。
- 不解决多生产者复杂配置仲裁；当前只有通信任务发布配置快照。

## 步骤

1. 创建活动计划和功能分支。
2. 扩展 task model、FreeRTOS context 和队列创建。
3. 扩展 `command_responder` 元数据。
4. 接入通信任务和环境处理任务。
5. 补充测试与文档。
6. 运行主机构建、测试和固件构建矩阵。
7. 提交、推送、开 PR，检查通过后合并。

## 验证方式

- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- 固件构建矩阵：
  - `cmake --build build-fw --clean-first`
  - `cmake --build build-fw-real-usart2-command --clean-first`
  - `cmake --build build-fw-rtos --clean-first`
  - `cmake --build build-fw-scheduler --clean-first`

## 未决问题

- 后续若需要更严格的实时性，可考虑 FreeRTOS queue set 或专门配置服务任务。

## 进度记录

- 2026-05-09：从已合并的环境配置安全更新后创建计划，准备接入 FreeRTOS 配置同步队列。
- 2026-05-09：新增 `RTOS_QUEUE_CONFIG_UPDATE`，扩展 `command_responder_status_t`，通信任务在 `SET` 成功后发布配置快照，环境处理任务消费配置并调用 `environment_processor_update_config`。
- 2026-05-09：补充 `command_responder` 和 `rtos_task_model` 测试，新增学习笔记和设计边界说明。
- 2026-05-09：主机构建、35 个主机测试、普通固件、USART2 命令固件、FreeRTOS 固件和 scheduler 固件构建均通过。
