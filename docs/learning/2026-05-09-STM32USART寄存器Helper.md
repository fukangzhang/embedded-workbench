# STM32 USART 寄存器 Helper

## 这一步做了什么？

本步骤新增 `stm32_usart`，它是后续真实 UART 接入前的寄存器级小模块。

当前支持：

- 配置基础 `8N1` 串口格式。
- 根据 APB 外设时钟和 baud rate 写 `BRR`。
- 打开接收 `RE`、发送 `TE` 和 USART 总开关 `UE`。
- 轮询 `RXNE` 判断是否收到 byte。
- 轮询 `TXE` 判断是否可以发送 byte。

## 为什么不是直接写完整 UART 驱动？

完整 UART 驱动会同时涉及很多东西：

- RCC 外设时钟。
- GPIO alternate function。
- USART 寄存器。
- 中断或轮询。
- buffer 管理。
- 和 `serial_command_service` 的连接。

如果一次全写，调试时很难知道问题来自哪一层。所以先把 USART 寄存器 helper 做小、做可测试。

## 代码怎么读？

建议按这个顺序：

1. `drivers/include/embedded_workbench/stm32_usart.h`
   - 看 `stm32_usart_registers_t` 里有哪些寄存器。
   - 看 `stm32_usart_config_t` 为什么需要 APB clock 和 baud rate。

2. `drivers/src/stm32_usart.c`
   - 看 `stm32_usart_configure_8n1` 如何先清 `UE`，再写 `BRR` 和格式位。
   - 看 `rx_ready/tx_ready` 如何读 `SR` 的状态位。
   - 看 `read_byte/write_byte` 为什么只处理低 8 bit。

3. `tests/test_stm32_usart.c`
   - 看测试如何用普通结构体模拟 USART 寄存器。
   - 看为什么要测试失败路径不改寄存器。

## 嵌入式知识点

`8N1` 是串口里非常常见的格式：

- `8`：8 个数据位。
- `N`：no parity，不使用奇偶校验。
- `1`：1 个停止位。

`BRR` 是 baud rate register。这里的计算使用：

```text
BRR ~= peripheral_clock_hz / baud_rate
```

例如 `16 MHz / 9600 ~= 1666.67`，四舍五入后写 `1667`。

注意 `peripheral_clock_hz` 是 USART 所在 APB 总线的时钟，不一定等于 CPU 主频。后续接真实板卡时，要确认 USART2 挂在哪条 APB 总线上，以及时钟树配置后的实际频率。

