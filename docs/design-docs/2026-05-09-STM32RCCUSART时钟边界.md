# STM32 RCC USART 时钟边界

## 背景

USART 寄存器和 GPIO alternate function 都准备好后，还缺一个前置条件：USART 外设时钟必须在 RCC 中打开。否则访问 USART 寄存器通常不会得到期望行为。

## 决策

新增 `stm32_rcc_usart_clock` driver：

- 使用 `peripheral -> enable bit` 表描述外设时钟。
- 调用者提供具体 APB enable register 地址。
- 驱动只负责置位，不写死 STM32F401RE 地址。
- 主机测试用普通变量模拟 `APB1ENR`。

## 边界

本模块负责：

- 查找 USART 外设名对应的 RCC enable bit。
- 设置 enable register 的对应 bit。
- 拒绝未知外设、非法 bit 和空指针。

本模块不负责：

- 判断 USART 在 APB1 还是 APB2。
- 提供 STM32F401RE 的真实 RCC 地址。
- 配置 GPIO AF 或 USART BRR/CR1。

## 后续

下一步可以把 STM32F401RE 的 USART2 绑定补上：

```text
APB1ENR address
  -> USART2 bit17
  -> USART2 base address
  -> PA2/PA3 AF7
```

