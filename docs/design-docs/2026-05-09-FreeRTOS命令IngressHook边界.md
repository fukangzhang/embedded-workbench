# FreeRTOS 命令 Ingress Hook 边界

## 背景

`serial_command_ingress` 和 `serial_command_ingress_pump` 已经能把 reader callback 里的字节转换成 `command_t`。FreeRTOS `communication_task` 已经能消费 `command_queue` 并生成响应。

缺口是：reader 字节流还没有进入 FreeRTOS command queue。

## 决策

在 `freertos_rtos_port_context_t` 中增加可选 reader hook：

```c
serial_command_ingress_pump_read_fn command_read;
void *command_read_context;
```

`communication_task` 在 hook 存在时初始化 `serial_command_ingress`，每轮最多读取 16 字节，并通过 submit callback 发送到 `command_queue`。

## 边界

- reader hook 可选，未配置时不改变原有 `command_queue` 行为。
- 本阶段不绑定具体 USART2 寄存器或串口适配器。
- ingress submit 使用非阻塞 `xQueueSend`，保持 communication task 不被入口层长时间卡住。
- reader 存在时，`xQueueReceive(command_queue)` 使用通信任务周期作为超时，避免永远阻塞后无法继续读取 reader。

## 不做什么

- 不把响应文本写回 USART2。
- 不新增任务。
- 不改变命令协议。
- 不声明真实板上串口 RTOS 闭环已经完成。

## 后续

- 在固件 FreeRTOS 路径里把 `command_read` 绑定到 `stm32_usart_serial_io_read_byte`。
- 增加 response writer hook，把 `response_queue` 输出写回 USART2。
- 做板上 scheduler + USART2 输入输出验证。
