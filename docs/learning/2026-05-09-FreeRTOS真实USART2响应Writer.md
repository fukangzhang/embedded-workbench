# FreeRTOS 真实 USART2 响应 Writer

## 这一步做了什么？

在 FreeRTOS port context 里新增一个可选 writer hook：

```c
freertos_rtos_port_response_write_fn response_write;
void *response_write_context;
```

打开固件开关后：

```text
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_RESPONSE_WRITER
```

`communication_task` 生成响应后，会把 `response_queue` 里的文本写到真实 STM32F401RE USART2。

## 响应路径

现在 FreeRTOS 串口命令路径可以分成输入和输出两半：

```text
输入：
USART2 RXNE/DR
  -> stm32_usart_serial_io_read_byte
  -> command_read hook
  -> serial_command_ingress_pump
  -> command_queue

处理：
command_queue
  -> command_responder
  -> response_queue

输出：
response_queue
  -> response_write hook
  -> stm32_usart_serial_io_write
  -> USART2 TXE/DR
```

这比裸机 `serial_command_service` 多了队列和任务边界。面试讲项目时，这正是 FreeRTOS 版本比裸机版本更有价值的地方：输入、处理、输出可以拆在任务和队列里演进。

## 为什么 writer hook 也是可选的？

因为主机测试和早期 FreeRTOS 构建仍然需要“只把响应留在队列里”的行为。

如果强制要求 writer 存在：

- 主机 fake port 会被真实串口细节污染。
- 没有真实板卡的固件构建会更难验证。
- `rtos_port_receive_response` 这种队列观察入口会被过早破坏。

所以 writer 为空时，响应仍然只留在 `response_queue`。writer 存在时，`communication_task` 会尝试把队列里的响应写出去。

## 为什么先 peek 再 receive？

写串口可能失败，例如 USART TXE 一直没 ready。

代码里先：

```c
xQueuePeek(response_queue, &response, 0)
```

再调用 writer。只有 writer 返回成功，才：

```c
xQueueReceive(response_queue, &response, 0)
```

这样做的意思是：如果串口暂时写不出去，这条响应不会被提前从队列里拿掉，下一轮通信任务还能重试。

## 对应代码

- `app/include/embedded_workbench/rtos_port_freertos.h`
- `app/src/rtos_port_freertos.c`
- `CMakeLists.txt`
- `.github/workflows/ci.yml`
- `firmware/src/main.c`
- `drivers/src/stm32_usart_serial_io.c`

## CI 验证了什么？

新增的 CI 组合同时打开 reader 和 writer：

```text
EW_USE_FREERTOS=ON
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER=ON
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_RESPONSE_WRITER=ON
```

它证明：

- FreeRTOS port 能编译 writer hook。
- 固件入口能把真实 USART2 绑定到 reader 和 writer。
- `stm32_usart_serial_io_write` 可以作为 FreeRTOS response writer 使用。

它仍然不证明真实板上已经收发成功。

## 还缺什么？

还需要真实板上验证：

1. 烧录 scheduler 固件。
2. 通过 ST-LINK VCP 发送 `STATUS?`、`CONFIG?`、`SET ...`。
3. 观察串口是否返回响应。
4. 观察告警输出是否跟随配置和样本变化。

完成这一步后，才可以在 README 里把“真实 FreeRTOS 串口命令闭环”标为板上验证完成。
