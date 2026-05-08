# BSP 目标板 Profile

## BSP 是什么？
BSP 是 Board Support Package，板级支持包。

在嵌入式项目里，BSP 通常负责这些内容：

- 时钟初始化
- GPIO 初始化
- UART/I2C/SPI 外设初始化
- 板载 LED、按键等资源映射
- 调试口、下载口、启动配置

它回答的问题是：这块板子有哪些资源，怎么把 MCU 外设和项目需要的功能接起来。

## 为什么先做 board profile？
现在还没有真正接入 STM32 HAL、启动文件和链接脚本。如果直接写硬件初始化，很容易被具体开发板和工具链卡住。

所以先做 `board_profile`：

- 记录目标板名称
- 记录 MCU 型号
- 记录调试器
- 记录连接器
- 记录控制台 UART
- 记录传感器总线
- 记录告警输出引脚占位

这相当于目标板接入前的硬件地图。

## 为什么推荐 NUCLEO-F401RE？
当前默认 profile 选择 `NUCLEO-F401RE`。原因是：

- ST 官方 Nucleo-64 开发板
- 集成 ST-LINK 调试器/编程器
- 支持 Arduino Uno V3 和 ST morpho 扩展连接
- STM32F401RE 属于 Cortex-M4，后续讲 FreeRTOS、任务调度、通信和传感器都比较合适

`NUCLEO-F103RB` 也保留为备选。它更接近经典 STM32F103 学习路线，但 F401RE 对后续 FreeRTOS 和项目展示更有余量。

## 为什么不是直接用很便宜的 Blue Pill？
`STM32F103C8T6 Blue Pill` 很常见，也便宜，但它通常需要额外 ST-LINK，板卡版本和晶振/USB 电路质量也更不统一。

对求职展示项目来说，Nucleo 板的优势是：

- 官方资料完整
- 调试链路统一
- 接线和扩展更清楚
- 更适合作为可复现项目说明

如果手头已经有 Blue Pill，也可以后续加一个 profile，但不建议第一版就绑定它。

## 当前 profile 不是最终引脚表
当前 profile 里的 `USART2`、`I2C1`、`PA5` 等是第一版逻辑映射，后续真正接板时还要根据：

- ST user manual
- 原理图
- CubeMX 生成结果
- 实际传感器模块接线

继续修正。

## 和架构分层的关系
当前推荐依赖方向是：

```text
bsp -> drivers -> app
```

更准确地说：

- `bsp/` 描述板子和 MCU 资源
- `drivers/` 使用 BSP 提供的底层能力适配外设
- `app/` 使用 drivers 提供的数据和命令接口实现业务逻辑

这能避免业务层到处出现“某个 GPIO 口具体是 PA5”这种硬件细节。

## 参考资料
- ST NUCLEO-F401RE 产品页：https://www.st.com/en/evaluation-tools/nucleo-f401re.html
- ST NUCLEO-F103RB 产品页：https://www.st.com.cn/zh/evaluation-tools/nucleo-f103rb.html
- STM32 Nucleo-64 user manual UM1724：https://www.st.com/resource/en/user_manual/dm00105823-nucleo-f401re-user-manual-stmicroelectronics.pdf
