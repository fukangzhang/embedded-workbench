# STM32 RCC USART 时钟

## 这一步做了什么？

本步骤新增 `stm32_rcc_usart_clock`，它负责打开 USART 外设时钟。

它的工作很简单：

```text
"USART2"
  -> 查表找到 enable bit
  -> 在 RCC APB enable register 里置位
```

## 为什么要打开 RCC clock？

STM32 的外设默认不一定有时钟。这样可以省电，也可以避免不用的外设乱跑。

所以使用 USART2 前通常要做三类初始化：

1. 打开 GPIOA clock，让 PA2/PA3 可配置。
2. 把 PA2/PA3 配成 AF7。
3. 打开 USART2 clock，再配置 USART2 自己的寄存器。

这一步只解决第 3 类里的“打开 USART2 clock”。

## 代码怎么读？

建议按这个顺序：

1. `drivers/include/embedded_workbench/stm32_rcc_usart_clock.h`
   - 看 `stm32_rcc_usart_clock_peripheral_t` 如何表达外设名和 enable bit。

2. `drivers/src/stm32_rcc_usart_clock.c`
   - 看 `find_peripheral_clock` 如何查表。
   - 看 `stm32_rcc_enable_usart_clock` 如何置位。

3. `tests/test_stm32_rcc_usart_clock.c`
   - 看 `apb1enr` 如何用普通变量模拟真实 RCC 寄存器。
   - 看重复开启为什么是幂等的。
   - 看未知外设和非法 bit 为什么不能改寄存器。

## 嵌入式知识点

GPIOA 这类 GPIO 端口通常在 AHB 总线；USART2 这类串口外设通常在 APB 总线。不同总线有不同的 RCC enable register。

因此项目里把 GPIO clock 和 USART clock 分成两个小 helper。它们形状很像，但使用的是不同的 enable register 和不同的外设表。

