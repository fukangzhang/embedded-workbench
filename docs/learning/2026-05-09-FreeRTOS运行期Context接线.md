# FreeRTOS 运行期 Context 接线

## 这一步做了什么？

前面 FreeRTOS 已经能创建任务和队列，但 scheduler 路径里还缺少运行时资源：

- `sensor_acquire_task` 需要一个 `sensor_source`
- `alarm_output_task` 需要一个 `alarm_output_sink`

这一步在 `firmware/src/main.c` 里给 FreeRTOS context 补上默认资源：

```text
sequence_sensor_source
  -> filtered_sensor_source
  -> sensor_source
  -> freertos_rtos_port_context.sensor_source

alarm_output_sink
  -> freertos_rtos_port_context.alarm_output_sink
```

## 对应代码

- `firmware/src/main.c`
- `drivers/include/embedded_workbench/sequence_sensor_source.h`
- `drivers/src/sequence_sensor_source.c`
- `drivers/include/embedded_workbench/filtered_sensor_source.h`
- `drivers/src/filtered_sensor_source.c`
- `app/include/embedded_workbench/rtos_port_freertos.h`
- `app/src/rtos_port_freertos.c`

## 为什么要接模拟传感器 source？

真实传感器驱动还没有接入。为了让 scheduler 构建路径更接近真实运行，先用一组固定样本：

```text
normal sample
轻微升高 sample
严重异常 sample
```

采集任务启动后，会通过 `sensor_acquisition` 周期性读取这组样本，再送入 `sensor_sample_queue`。现在这组样本会先经过 `filtered_sensor_source`，所以轻微升高不会立刻造成剧烈跳变；但严重异常仍然会进入 alarm。

这仍然不是板上运行证明，但它让 FreeRTOS task 数据流具备了默认输入。

## 为什么滤波放在 context 装配处？

FreeRTOS task 不需要知道“这个 source 是原始的还是滤波后的”。它只依赖：

```c
sensor_source_t *sensor_source;
```

所以滤波层放在 `firmware/src/main.c` 里组合：

```text
raw sequence source
  -> filtered source
  -> context.sensor_source
```

这样做的意义是：以后换成真实传感器 source 时，仍然可以用同样方式选择是否包滤波层，而不需要改任务内部逻辑。

## 为什么 output sink 也要放进 context？

`alarm_output_task` 不应该知道具体是 board fake、STM32 GPIO 还是别的输出后端。

它只从 context 拿到：

```c
alarm_output_sink_t *alarm_output_sink;
```

这样输出任务只负责消费告警事件和刷新输出，真实落地方式仍由 sink 层决定。

## scheduler 启动条件为什么更保守？

现在 scheduler 只有在 `firmware_self_check == 1` 时才尝试启动。

原因是 `vTaskStartScheduler()` 成功后通常不会返回。如果基础自检已经失败，还继续启动调度器，会让失败定位更乱。

所以顺序变成：

```text
基础自检通过
  -> 准备 FreeRTOS context
  -> 创建任务
  -> 显式开关允许时启动 scheduler
```

## 看完要能解释什么？

- `freertos_rtos_port_context_t` 为什么保存 task 运行期资源
- 为什么 scheduler 路径现在有默认模拟输入
- 为什么滤波层放在装配处，而不是写死进 FreeRTOS task
- 为什么输出任务通过 sink 间接操作硬件输出
- 为什么基础自检失败时不应该继续启动调度器
