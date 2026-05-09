# 串口命令 Ingress Pump

## 这一步做了什么？

新增：

```text
serial_command_ingress_pump
```

它负责从 reader callback 里有界读取字节，并把这些字节喂给 `serial_command_ingress`：

```text
reader callback
  -> serial_command_ingress_pump_poll
  -> serial_command_ingress_feed
  -> submit(command_t)
```

## 对应代码

- `app/include/embedded_workbench/serial_command_ingress_pump.h`
- `app/src/serial_command_ingress_pump.c`
- `tests/test_serial_command_ingress_pump.c`
- `CMakeLists.txt`

## 为什么 ingress 已经有 feed，还要 pump？

`serial_command_ingress_feed` 一次只处理一个字节。真实固件里通常需要反复读取 USART：

```text
while (USART has data) {
    feed one byte
}
```

如果每个任务都自己写这个循环，容易出现重复逻辑：

- 最多读多少字节？
- 收到完整命令后是否继续读？
- 溢出或错误时如何停？
- 这一轮到底读了多少字节？

`serial_command_ingress_pump` 把这层轮询外壳集中起来。

## 为什么要限制 max_bytes？

在 FreeRTOS task 里，一个任务不能无限读串口，否则可能长时间占用 CPU，让别的任务得不到运行机会。

`max_bytes` 表示这一轮最多读取多少字节：

```c
serial_command_ingress_pump_poll(..., max_bytes, &bytes_read);
```

如果没读完，下一轮 task 再继续读。

## 什么时候会停下？

pump 遇到这些状态会立即返回：

- `COMMAND_SUBMITTED`：已经提交一条命令，后续响应链路可以工作。
- `OVERFLOW`：当前行太长，调用方可以记录或忽略。
- `ERROR`：reader、ingress 或 submit 出错。

如果没有数据，或者只读到半行，就返回 `IDLE`。

## 和 serial_command_pump 的区别

`serial_command_pump` 面向 `serial_command_service`：

```text
byte -> line -> command_session -> response writer
```

`serial_command_ingress_pump` 面向 `serial_command_ingress`：

```text
byte -> line -> command_t submit
```

前者适合裸机同步响应，后者适合后续 FreeRTOS command queue。

## 测试覆盖了什么？

`tests/test_serial_command_ingress_pump.c` 覆盖：

- reader 没数据时返回 idle，读取 0 字节。
- `max_bytes` 会限制每轮工作量。
- 收到完整命令后提交并停在换行处，不吞后续字节。
- 下一轮 poll 能继续处理上轮没读完的命令。
- 行溢出时返回 overflow。
- 空参数和 `max_bytes=0` 返回 error。

## 看完要能解释什么？

- feed 和 pump 的职责区别
- 为什么 FreeRTOS 任务需要有界读取
- 为什么 pump 在提交一条命令后要停下
- 它如何为后续 USART2 到 `command_queue` 接线铺路
