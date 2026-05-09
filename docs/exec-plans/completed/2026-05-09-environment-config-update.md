# Environment Config Update

## 目标

为 `environment_processor` 增加安全更新配置的能力，让后续 FreeRTOS 通信任务可以把 `SET` 命令产生的新阈值同步给环境处理任务。

## 范围

- 新增 `environment_processor_update_config`。
- 验证合法配置会被复制到 processor。
- 验证非法配置、空指针不会污染旧配置。
- 补充设计说明和学习文档。

## 不在本次范围

- 不新增 FreeRTOS 配置队列。
- 不把通信任务的 SET 结果同步给环境处理任务。
- 不改变 `command_responder` 行为。
- 不启动真实板上调度器验证。

## 步骤

1. 创建活动计划和功能分支。
2. 扩展 `environment_processor` 接口和实现。
3. 扩展 `tests/test_environment_processor.c`。
4. 更新设计说明、学习文档和 README/索引。
5. 运行主机构建、测试和固件构建矩阵。
6. 提交、推送、开 PR，检查通过后合并。

## 验证方式

- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- 固件构建矩阵：
  - `cmake --build build-fw --clean-first`
  - `cmake --build build-fw-real-usart2-command --clean-first`
  - `cmake --build build-fw-rtos --clean-first`
  - `cmake --build build-fw-scheduler --clean-first`

## 未决问题

- 后续需要设计通信任务到环境任务的配置同步消息通道。

## 进度记录

- 2026-05-09：从已合并的命令响应步骤后创建计划，准备补环境处理器配置更新能力。
- 2026-05-09：新增 `environment_processor_update_config`，合法配置会复制进 processor，非法配置会被拒绝且旧配置保持生效。
- 2026-05-09：补充测试、设计说明、学习文档和 README 学习路线。
- 2026-05-09：本地验证通过：主机构建、35 个 CTest、4 组固件 clean build。
