# Environment Processor Step

## 目标

新增一个可在主机测试的环境处理步骤模块，把“收到一帧传感器样本”转换为“更新后的告警状态事件”。

## 范围

- 新增 app 层 `environment_processor` 接口和实现。
- 使用 fake publish 回调验证正常状态更新、告警状态更新、非法输入、发布失败等路径。
- 让 FreeRTOS `env_process_task` 复用这个步骤，而不是直接在任务里写完整状态更新和事件发送逻辑。
- 补充设计说明、学习文档和 README 学习路线。

## 不在本次范围

- 不处理串口命令对阈值的实时修改。
- 不启动真实 FreeRTOS 调度器做板上验证。
- 不改变 `alarm_state` 阈值算法。

## 步骤

1. 创建活动计划和功能分支。
2. 新增 `environment_processor` 头文件、源文件和 CMake 接线。
3. 更新 FreeRTOS 环境处理任务，使用新模块。
4. 新增主机单元测试。
5. 更新学习文档、设计文档和 README。
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

- 后续通信任务如何把 SET 阈值命令同步给环境处理任务，需要单独设计，不混在本次变更里。

## 进度记录

- 2026-05-09：从已合并的采集步骤后创建计划，准备抽出环境处理步骤。
- 2026-05-09：新增 `environment_processor` 模块、主机测试和 FreeRTOS 环境处理任务接线。
- 2026-05-09：补充设计说明、学习文档和 README 学习路线。
- 2026-05-09：本地验证通过：主机构建、34 个 CTest、4 组固件 clean build。
