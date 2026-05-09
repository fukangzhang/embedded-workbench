# STM32 USART 寄存器 Helper 边界

## 背景

项目已经有 `serial_command_service`，它负责把 byte 流接到命令会话。但真实 STM32 UART 还需要一层硬件寄存器操作：

- 设置 baud rate。
- 打开 receiver/transmitter。
- 轮询 RXNE/TXE。
- 从 DR 读 byte 或向 DR 写 byte。

这一步先做最小 USART helper，不直接上完整 USART2 驱动。

## 决策

新增 `stm32_usart` driver 模块：

- 用 `stm32_usart_registers_t` 表达 USART 寄存器。
- 支持 8N1、oversampling by 16 的基础配置。
- `BRR` 使用 `peripheral_clock_hz / baud_rate` 四舍五入计算。
- 提供 polling 风格的 `rx_ready`、`tx_ready`、`read_byte`、`write_byte`。
- 保持主机可测试，测试里用普通结构体模拟寄存器。

## 边界

本模块负责：

- 写 `BRR/CR1/CR2/CR3` 中当前项目需要的字段。
- 检查无效 clock、baud、空指针和 TX/RX 都关闭的配置。
- 从 `SR` 判断 RX/TX 是否就绪。

本模块不负责：

- 打开 USART 外设 RCC 时钟。
- 配置 PA2/PA3 的 GPIO alternate function。
- 绑定 USART2 的真实地址。
- 中断、DMA、ring buffer。
- 调用 `serial_command_service`。

## 后续

下一步可以新增板级 USART2 绑定与初始化链路：

```text
RCC USART2 clock
  -> PA2/PA3 alternate function AF7
  -> stm32_usart_configure_8n1
  -> polling/interrupt byte bridge
  -> serial_command_service
```

