# FreeRTOS 真实 USART2 命令 Reader 边界

## 背景

FreeRTOS command ingress hook 已经存在，但还没有绑定真实 USART2 reader。裸机 USART2 command loop 已经能同步读写，但它不适合直接代表 FreeRTOS 任务路径。

## 决策

新增默认关闭开关：

```text
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER
```

打开后，固件 FreeRTOS runtime context 初始化真实 USART2，并设置：

```c
command_read = stm32_usart_serial_io_read_byte;
command_read_context = &firmware_freertos_usart2_serial_io;
```

## 边界

- 本阶段只接输入端 reader，不接 `response_queue` 到 USART2 writer。
- 使用独立 `firmware_freertos_usart2_serial_io`，避免被模拟自检覆盖。
- 构建开关默认关闭，只有明确打开时才解引用真实 STM32 外设地址。
- CI 只验证构建和链接，不声明真实板上运行成功。

## 不做什么

- 不改变串口命令协议。
- 不替换裸机 USART2 command loop。
- 不启动真实板上 scheduler。

## 后续

- 增加 FreeRTOS response writer hook，把 response queue 文本写回 USART2。
- 增加组合构建或板上验证记录，覆盖 reader + writer + scheduler。
