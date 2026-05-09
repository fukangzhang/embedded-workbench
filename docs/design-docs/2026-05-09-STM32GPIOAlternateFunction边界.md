# STM32 GPIO Alternate Function 边界

## 背景

USART2 的 TX/RX 不能只配置 USART 寄存器，还必须把对应 GPIO pin 切到 alternate function。以 NUCLEO-F401RE 常见串口为例，PA2/PA3 需要配置为 AF7 才能连接 USART2。

项目已有 `stm32_gpio_configure_output`，但它只支持普通输出模式。本步骤扩展同一个 GPIO 配置模块，增加 alternate function 配置。

## 决策

在 `stm32_gpio_config` 中新增：

- `stm32_gpio_alternate_config_t`
- `stm32_gpio_alternate_config_default`
- `stm32_gpio_configure_alternate_function`

同时把 `stm32_gpio_registers_t` 扩展到包含 `AFRL/AFRH`，并保留真实 STM32 GPIO 寄存器顺序中间的 `IDR/ODR/BSRR/LCKR` 字段。

## 边界

本模块负责：

- 把 `MODER` 设置为 alternate function 模式。
- 配置 `OTYPER/OSPEEDR/PUPDR`。
- 根据 pin 号写 `AFRL` 或 `AFRH` 的 4-bit AF 字段。
- 校验 pin、port、AF 编号和电气配置。

本模块不负责：

- 判断某个 pin 是否真的支持某个 AF。
- 打开 GPIO 端口 RCC 时钟。
- 打开 USART 外设时钟。
- 调用 USART helper。

## 后续

USART2 板级初始化可以按下面顺序推进：

```text
enable GPIOA clock
  -> configure PA2/PA3 alternate function AF7
  -> enable USART2 clock
  -> stm32_usart_configure_8n1
  -> serial byte bridge
```

