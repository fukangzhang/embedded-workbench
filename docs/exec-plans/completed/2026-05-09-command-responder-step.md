# Command Responder Step

## 目标

新增一个可在主机测试的命令响应步骤模块，把已经解析好的 `command_t` 处理成响应文本，使 FreeRTOS `communication_task` 不再只返回固定 `OK`。

## 范围

- 新增 app 层 `command_responder` 接口和实现。
- 复用现有 `command_handler` 和 `response_format`。
- 支持 STATUS、CONFIG、SET、SAMPLE、CLEAR_ALARM 和非法命令响应。
- 让 FreeRTOS `communication_task` 复用该模块。
- 补充主机单元测试、设计说明、学习文档和 README 学习路线。

## 不在本次范围

- 不改文本命令 parser。
- 不改串口逐字节服务。
- 不解决 FreeRTOS 运行时配置跨任务同步。
- 不启动真实板上调度器验证。

## 步骤

1. 创建活动计划和功能分支。
2. 新增 `command_responder` 头文件、源文件和 CMake 接线。
3. 更新 FreeRTOS `communication_task`，使用 responder 生成响应。
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

- FreeRTOS 通信任务和环境处理任务各自持有配置/状态时，后续需要设计一致性同步方式。

## 进度记录

- 2026-05-09：从已合并的环境处理步骤后创建计划，准备替换通信任务固定 OK 骨架。
- 2026-05-09：新增 `command_responder` 模块、主机测试和 FreeRTOS 通信任务接线。
- 2026-05-09：把 `rtos_response_message_t` 文本空间显式扩展到 512，并让 task model 使用真实结构体大小。
- 2026-05-09：补充设计说明、学习文档和 README 学习路线。
- 2026-05-09：本地验证通过：主机构建、35 个 CTest、4 组固件 clean build。
