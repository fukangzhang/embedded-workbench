# STM32 GPIO Alternate Function

## 这一步做了什么？

本步骤让 `stm32_gpio_config` 支持 alternate function 配置。后续 USART2 要使用 PA2/PA3 时，必须先把这两个 GPIO pin 从普通 GPIO 模式切换到 AF7。

## 什么是 alternate function？

STM32 的一个引脚可以有多种用途。比如 PA2 既可以是普通 GPIO，也可以连接到 USART2_TX。选择哪一种用途，不是由 USART 模块单独决定的，而是由 GPIO 的 alternate function 寄存器决定。

简化理解：

```text
PA2 普通 GPIO
  或
PA2 -> AF7 -> USART2_TX
```

## 代码怎么读？

建议按这个顺序：

1. `drivers/include/embedded_workbench/stm32_gpio_config.h`
   - 看 `stm32_gpio_registers_t` 为什么现在包含 `AFRL/AFRH`。
   - 看 `stm32_gpio_alternate_config_t` 里为什么既有 AF 编号，也有 speed/pull/output type。

2. `drivers/src/stm32_gpio_config.c`
   - 看 `write_four_bit_field` 如何写 4-bit AF 字段。
   - 看 `stm32_gpio_configure_alternate_function` 如何选择 `AFRL` 或 `AFRH`。

3. `tests/test_stm32_gpio_config.c`
   - 看 PA2 为什么写 `AFRL`。
   - 看 PA9 为什么写 `AFRH`。
   - 看失败路径为什么要确认已有寄存器不变。

## 嵌入式知识点

GPIO mode 和 alternate function 是两层配置：

- `MODER` 选择这个 pin 是 input/output/alternate/analog。
- `AFRL/AFRH` 选择 alternate 模式下连接哪个外设功能。

所以 USART2 初始化时不能只写 USART 的 `CR1/BRR`。如果 PA2/PA3 还停留在普通 GPIO 模式，USART2 即使启动了，也不会正确连到引脚。

