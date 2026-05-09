# FreeRTOS 命令 Ingress Hook

## 这一步做了什么？

FreeRTOS 的 `communication_task` 原本只消费已经放进 `command_queue` 的 `command_t`。

这一步给 FreeRTOS context 增加了一个可选 reader hook：

```c
serial_command_ingress_pump_read_fn command_read;
void *command_read_context;
```

配置 reader 后，`communication_task` 会：

```text
reader callback
  -> serial_command_ingress_pump
  -> serial_command_ingress
  -> command_queue
  -> command_responder
  -> response_queue
```

如果没有配置 reader，通信任务保持原有行为：继续只阻塞等待 `command_queue`。

## 对应代码

- `app/include/embedded_workbench/rtos_port_freertos.h`
- `app/src/rtos_port_freertos.c`
- `app/include/embedded_workbench/serial_command_ingress.h`
- `app/include/embedded_workbench/serial_command_ingress_pump.h`

## 为什么 reader hook 是可选的？

真实 USART2 reader 已经可以通过默认关闭的 `EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER` 开关接入 FreeRTOS 路径。

如果强行要求 reader 必须存在，就会破坏当前已有的 FreeRTOS 构建和队列自检。可选 hook 的好处是：

- 没有 reader：测试或固件仍然可以通过 `rtos_port_send_command` 投递命令。
- 有 reader：communication task 可以主动从串口来源读取字节。

## 为什么不能一直阻塞在 command_queue？

如果配置了 reader，communication task 需要周期性 poll reader。

所以在 reader 可用时，接收 `command_queue` 使用通信任务周期作为超时：

```text
xQueueReceive(command_queue, timeout=communication period)
```

这样它既能处理队列里的命令，也不会因为永久阻塞而忘记读取串口字节。

没有 reader 时仍然可以 `portMAX_DELAY` 永久等待队列，节省调度开销。

## ingress 提交命令怎么进队列？

`serial_command_ingress` 的 submit callback 在 FreeRTOS port 中绑定到：

```text
freertos_submit_ingress_command
  -> xQueueSend(command_queue)
```

这让串口输入和其他命令来源都汇合到同一个 `command_queue`，后续处理路径保持统一。

## 当前还不是完整真实串口闭环

这一层已经把 hook 接到 communication task，并且后续补了真实 USART2 reader 绑定。

还缺：

- 把 `response_queue` 的文本真正写回 USART2。
- 板上启动 scheduler 后观察串口输入/输出。

## 看完要能解释什么？

- reader hook 为什么放在 `freertos_rtos_port_context_t`
- 为什么 reader 存在时不能永久阻塞在 `command_queue`
- `serial_command_ingress_pump` 如何把字节变成 `command_t`
- 为什么所有命令最终还是走同一个 `command_queue`
