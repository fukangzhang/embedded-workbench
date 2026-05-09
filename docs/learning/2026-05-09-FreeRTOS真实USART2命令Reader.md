# FreeRTOS 真实 USART2 命令 Reader

## 这一步做了什么？

新增一个默认关闭的固件构建开关：

```text
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER
```

打开后，FreeRTOS 固件路径会初始化真实 STM32F401RE USART2，并把 USART2 单字节读取函数绑定到：

```c
freertos_rtos_port_context_t.command_read
```

数据入口变成：

```text
USART2 RXNE/DR
  -> stm32_usart_serial_io_read_byte
  -> command_read hook
  -> serial_command_ingress_pump
  -> serial_command_ingress
  -> command_queue
```

## 对应代码

- `CMakeLists.txt`
- `.github/workflows/ci.yml`
- `firmware/src/main.c`
- `app/include/embedded_workbench/rtos_port_freertos.h`
- `app/src/rtos_port_freertos.c`
- `drivers/include/embedded_workbench/stm32_usart_serial_io.h`

## 为什么要新增独立开关？

已有的：

```text
EW_FIRMWARE_USE_REAL_STM32_USART2_COMMAND_LOOP
```

是裸机轮询命令 loop，用 `serial_command_service` 同步读命令并写响应。

新的：

```text
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER
```

只负责 FreeRTOS 输入侧，把 USART2 字节送进 `command_queue`。它不负责把 `response_queue` 写回 USART2。

两个路径用途不同，所以用不同开关更清楚。

## 为什么要用独立 serial I/O context？

`firmware/src/main.c` 里已有一个 `firmware_usart2_serial_io`，它会被模拟 USART2 自检使用。

如果 FreeRTOS reader 复用它，真实 USART2 context 可能在 scheduler 启动前被模拟自检覆盖。

所以这一步新增独立 context：

```c
firmware_freertos_usart2_serial_io
```

这样 FreeRTOS reader 始终指向真实 USART2 寄存器绑定。

## CI 验证了什么？

CI 新增了一个构建组合：

```text
EW_USE_FREERTOS=ON
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER=ON
```

它验证：

- FreeRTOS port 能引用 command reader hook。
- 固件入口能初始化真实 USART2 reader 路径。
- 真实 STM32 地址绑定路径能编译链接。

它仍然不证明板上串口已经收到了字节。

## 还缺什么？

当前只完成输入端 reader：

```text
USART2 -> command_queue
```

还缺输出端 writer：

```text
response_queue -> USART2
```

完成 writer 后，才更接近真实 FreeRTOS 串口命令闭环。最后仍需要板上启动 scheduler 并用 ST-LINK VCP 观察输入输出。

## 看完要能解释什么？

- 为什么 FreeRTOS USART2 reader 和裸机 USART2 command loop 分开开关
- 为什么不能复用模拟自检用的 USART2 serial I/O context
- 当前 CI 证明的是构建和链接，不是板上运行
- 为什么下一步应该做 response writer hook
