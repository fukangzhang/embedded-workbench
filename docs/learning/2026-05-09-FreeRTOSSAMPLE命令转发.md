# FreeRTOS SAMPLE 命令转发

## 这一步做了什么？

`SAMPLE` 命令原来已经能更新 `command_responder` 持有的样本，并生成新的 `STATUS` 响应。

这一步补上 FreeRTOS 路径里的队列转发：

```text
SAMPLE ...
  -> command_responder
  -> status.sample_changed = true
  -> communication_task
  -> sensor_sample_queue
  -> env_process_task
```

这样手工注入样本不只影响串口响应，也会进入环境处理任务。

## 为什么要这样做？

FreeRTOS 版本里，环境处理任务只关心一个入口：

```text
sensor_sample_queue
```

真实传感器采集任务会往这个队列送样本。`SAMPLE` 命令作为调试入口，也应该走同一条队列，否则它就只是“通信任务内部状态变化”，不会推动告警事件流。

## 对应代码

- `app/src/rtos_port_freertos.c`
- `app/include/embedded_workbench/command_responder.h`
- `app/src/command_responder.c`
- `docs/design-docs/2026-05-09-FreeRTOSSAMPLE命令转发边界.md`

核心判断是：

```c
if (status.sample_changed) {
    (void)freertos_submit_sensor_sample(context, responder.sample);
}
```

## 队列满了怎么办？

当前代码忽略投递失败：

```c
(void)freertos_submit_sensor_sample(...)
```

这是一个早期阶段的取舍。串口响应仍然代表命令解析和 responder 更新成功，但如果 `sensor_sample_queue` 已满，环境处理任务可能暂时看不到这次样本。

后续如果要更适合产品化，可以把这种失败变成 response warning 或 telemetry 事件。

## 看完要能解释什么？

- 为什么 FreeRTOS 里的 `SAMPLE` 要进入 `sensor_sample_queue`
- 为什么不新增第二条 sample 队列
- 为什么当前构建验证不等于真实板上告警输出已经跟着变化
