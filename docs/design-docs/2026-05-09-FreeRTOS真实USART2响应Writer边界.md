# FreeRTOS 真实 USART2 响应 Writer 边界

## 背景

FreeRTOS USART2 command reader 已经能把真实串口输入送入 `command_queue`，但响应仍然停留在 `response_queue`，没有写回真实 USART2。

## 决策

新增默认关闭开关：

```text
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_RESPONSE_WRITER
```

打开后，固件 FreeRTOS runtime context 会设置：

```c
response_write = stm32_usart_serial_io_write;
response_write_context = &firmware_freertos_usart2_serial_io;
```

`communication_task` 生成响应并放入 `response_queue` 后，会尝试通过 writer hook 写回 USART2。

## 边界

- writer hook 可选，默认不影响主机测试和早期 FreeRTOS 队列自检。
- 写出逻辑只在 `communication_task` 中触发，不新增独立 response task。
- 写出前使用 `xQueuePeek`，写出成功后再 `xQueueReceive` 移除响应。
- reader 和 writer 复用同一个 `firmware_freertos_usart2_serial_io`，因为它们指向同一个真实 USART2 寄存器块。
- CI 只验证构建和链接，不声明真实板上收发成功。

## 不做什么

- 不引入 USART 中断、DMA 或环形 TX buffer。
- 不改变命令响应文本格式。
- 不把裸机 USART2 command loop 替换成 FreeRTOS 路径。

## 后续

- 在 NUCLEO-F401RE 上启动 scheduler，使用 ST-LINK VCP 验证命令输入和响应输出。
- 若轮询写串口导致任务阻塞，再引入 TX queue、独立 writer task 或中断/DMA。
