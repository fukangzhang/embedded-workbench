# FreeRTOS 运行期 Context 接线边界

## 背景

FreeRTOS port 已经能创建任务、队列和 scheduler handoff 路径。为了让 scheduler 构建更接近真实任务运行，需要在启动前给 context 放入任务运行期依赖。

## 决策

在固件 FreeRTOS 路径中初始化：

- `sequence_sensor_source_t`：作为临时模拟传感器输入。
- `filtered_sensor_source_t`：包装临时模拟输入，让 context 接收到滤波后的 source。
- `sensor_source_t`：写入 `freertos_rtos_port_context_t.sensor_source`。
- `alarm_output_sink_t`：写入 `freertos_rtos_port_context_t.alarm_output_sink`。

同时，scheduler 只在 `firmware_self_check == 1` 时启动。

## 边界

- 这仍然是构建级和固件入口级接线，不声明真实板上 scheduler 已经跑通。
- 模拟样本用于让采集任务有输入，不替代真实传感器驱动。
- 滤波层只在 `firmware/src/main.c` 装配处启用，不改变 `app/src/rtos_port_freertos.c` 的任务逻辑。
- 当前滤波参数为 `1/2`，用于 demo；它不是最终产品参数。
- 输出 sink 指针复用已有告警输出链路，不让 FreeRTOS task 直接依赖 STM32 GPIO。
- 基础自检失败时不进入 scheduler，保留 `firmware_self_check` 作为失败定位信号。

## 后续

- 接真实 sensor driver 后，可以把 `sequence_sensor_source` 替换成真实 source。
- 真实传感器 source 可以继续复用 `filtered_sensor_source`，但 alpha 参数来源需要单独设计。
- 板上验证时需要记录 SysTick、SVC/PendSV、任务栈和告警输出观察结果。
