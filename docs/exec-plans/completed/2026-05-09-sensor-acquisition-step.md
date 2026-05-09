# Sensor Acquisition Step

## 目标

新增一个可在主机测试的采集步骤模块，把“从 `sensor_source_t` 读取一帧样本”与“把样本提交给后续管道”连接起来。

## 范围

- 新增 app 层 `sensor_acquisition` 接口和实现。
- 使用 fake submit 回调验证成功读取、读取失败、提交失败、非法输入等路径。
- 让 FreeRTOS `sensor_acquire_task` 复用这个步骤，而不是在任务里直接写完整采集逻辑。
- 补充设计说明、学习文档和 README 学习路线。

## 不在本次范围

- 不接真实 I2C/ADC 传感器。
- 不启动真实 FreeRTOS 调度器做板上验证。
- 不改变串口命令协议。

## 步骤

1. 创建活动计划和功能分支。
2. 新增 `sensor_acquisition` 头文件、源文件和 CMake 接线。
3. 更新 FreeRTOS 采集任务，使用新模块。
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

- 真实传感器 source 后续采用 I2C 还是 ADC，需要结合板卡外设和传感器选型再决定。

## 进度记录

- 2026-05-09：从已合并的序列传感器来源 PR 后创建计划，准备实现 app 层采集步骤。
- 2026-05-09：新增 `sensor_acquisition` 模块、主机测试和 FreeRTOS 采集任务接线。
- 2026-05-09：补充设计说明、学习文档和 README 学习路线。
- 2026-05-09：本地验证通过：主机构建、33 个 CTest、4 组固件 clean build。
