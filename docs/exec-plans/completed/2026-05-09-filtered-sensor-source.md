# 2026-05-09 滤波传感器来源

## 目标

把已经完成的 `sensor_sample_filter` 接到现有 `sensor_source` 抽象上，新增一个可组合的滤波传感器来源。

## 范围

- 新增 `filtered_sensor_source` 驱动模块。
- 新增主机测试覆盖初始化、首帧透传、后续样本滤波、内层读取失败和 reset。
- 更新学习笔记、设计说明和学习索引。
- 不改 `host_sim`、FreeRTOS 运行链路或真实硬件默认行为。

## 步骤

1. 阅读现有 `sensor_source`、`sequence_sensor_source` 和 `sensor_sample_filter` 接口。
2. 增加 `filtered_sensor_source` 头文件和实现。
3. 将新模块加入 `embedded_drivers` 构建。
4. 增加 `test_filtered_sensor_source`。
5. 更新文档，说明它在代码链路中的位置和暂不接入运行时的原因。
6. 运行主机构建、测试、固件构建和格式检查。

## 验证方式

- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- `git diff --check`
- `cmake --build build-fw --clean-first`
- `cmake --build build-fw-scheduler-real-usart2-io --clean-first`

## 未决问题

- 真实传感器接入后，需要决定滤波参数来自编译期配置、串口命令还是持久化配置。
- 本任务只提供组合层，不改变当前告警判断输入来源。

## 进度记录

- 2026-05-09：创建计划，开始实现。
- 2026-05-09：新增 `filtered_sensor_source`、主机测试、学习笔记和设计边界。
- 2026-05-09：`cmake --build build`、`ctest --test-dir build --output-on-failure`、`git diff --check`、`cmake --build build-fw --clean-first`、`cmake --build build-fw-scheduler-real-usart2-io --clean-first` 均通过。
