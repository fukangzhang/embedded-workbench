# 串口命令 Ingress Pump 边界

## 背景

`serial_command_ingress` 已经能把单个字节流转成 `command_t` submit 回调。后续接 USART2 或 FreeRTOS task 时，还需要一个有界读取外壳，避免每个调用点重复写 reader 循环。

## 决策

新增 `serial_command_ingress_pump`：

```text
reader callback
  -> pump poll
  -> serial_command_ingress_feed
```

## 边界

- pump 不解析命令，只调用 `serial_command_ingress_feed`。
- pump 不写响应，只返回 ingress status。
- pump 用 `max_bytes` 限制单轮读取量。
- pump 在 `COMMAND_SUBMITTED`、`OVERFLOW` 或 `ERROR` 时立即停止。
- reader callback 只表达“本轮是否读到一个字节”，不暴露具体 USART 寄存器。

## 不做什么

- 不替换 `serial_command_pump`。
- 不直接操作 STM32 USART2。
- 不直接依赖 FreeRTOS。

## 后续

后续 FreeRTOS USART2 接入可以把 reader callback 绑定到 `stm32_usart_serial_io_read_byte`，把 ingress submit callback 绑定到 `command_queue` 发送。
