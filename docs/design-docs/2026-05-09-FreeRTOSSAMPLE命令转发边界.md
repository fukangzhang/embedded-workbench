# FreeRTOS SAMPLE 命令转发边界

## 背景

`SAMPLE` 命令已经能让 `command_responder` 更新通信任务内部的样本，并返回新的 `STATUS`。但在 FreeRTOS 路径里，环境处理任务只消费 `sensor_sample_queue`，如果 `SAMPLE` 不进入这个队列，告警事件流不会看到这次手工注入。

## 决策

当 `command_responder_handle_command_with_status` 返回：

```c
status.sample_changed == true
```

`communication_task` 会把 `responder.sample` 投递到：

```text
sensor_sample_queue
```

这样 `SAMPLE` 命令和真实采集任务产生的样本共享同一个环境处理入口。

## 边界

- 不新增 `sample_update_queue`，直接复用已有 `sensor_sample_queue`。
- 队列满时本次转发会失败，但响应仍按命令处理结果返回；后续真实板上观察时需要关注是否有数据生产过快的问题。
- 不改变 `SAMPLE` 命令文本格式。
- 不改变 `command_responder` 的纯逻辑职责，它仍只负责更新 responder 持有的状态并报告 `sample_changed`。

## 后续

- 板上 VCP 验证时，发送 `SAMPLE 360 600 250 20` 后观察串口响应和告警输出。
- 如果后续需要更强保证，可以把队列满作为响应 warning 或 telemetry 事件暴露出来。
