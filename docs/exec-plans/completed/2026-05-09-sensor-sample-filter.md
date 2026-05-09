# Sensor Sample Filter

## 目标

新增一个主机可测试的传感器样本滤波模块，用固定点整数方式平滑 `sensor_sample_t`，为后续真实传感器接入做准备。

## 范围

- 新增 `sensor_sample_filter` driver 模块。
- 使用整数权重实现一阶平滑，不引入浮点。
- 新增主机测试覆盖初始化、首样本、连续滤波、重置和非法输入。
- 更新学习文档和 README/学习索引。

## 不在本次范围

- 不接入 `sensor_acquisition` 或 FreeRTOS 任务。
- 不改变现有告警状态机行为。
- 不引入真实传感器驱动。

## 验证方式

- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`

## 未决问题

- 后续需要决定滤波应接在采集任务内，还是环境处理任务前。

## 进度记录

- 2026-05-09：创建计划，准备新增整数样本滤波模块。
- 2026-05-09：新增 `sensor_sample_filter` driver 模块，支持整数权重、首样本透传、连续滤波和 reset。
- 2026-05-09：新增 `tests/test_sensor_sample_filter.c`，主机测试从 37 个增加到 38 个。
- 2026-05-09：补充设计边界、学习文档和 README/学习索引。
- 2026-05-09：`cmake --build build`、38 个主机测试、`git diff --check`、基础固件和 scheduler+USART2 I/O 固件构建均通过。
