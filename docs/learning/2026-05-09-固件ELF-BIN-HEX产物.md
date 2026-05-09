# 固件 ELF/BIN/HEX 产物

## 这一步做了什么？

固件构建现在不只生成：

```text
embedded_firmware.elf
```

还会自动生成：

```text
embedded_firmware.bin
embedded_firmware.hex
```

## 三种文件有什么区别？

`ELF` 包含代码、数据、符号等调试信息，适合调试器加载、查看函数名、定位地址。

`BIN` 是裸二进制镜像，适合一些烧录工具直接写入 flash。使用它时通常需要知道起始 flash 地址，比如 STM32F401RE 的 `0x08000000`。

`HEX` 是 Intel HEX 文本格式，每行带地址和校验信息，很多 MCU 烧录工具都支持。

## 对应代码

- `CMakeLists.txt`
- `cmake/toolchains/zig-arm-none-eabi.cmake`
- `hardware/nucleo-f401re-bringup.md`

## 为什么要处理 Zig 和 GNU 的差异？

GNU objcopy 生成 Intel HEX 通常用：

```text
-O ihex
```

Zig 自带的 objcopy 使用：

```text
-O hex
```

项目当前 CI 和本地固件构建主要使用 Zig 路径，所以 CMake 里根据可用工具选择合适格式名。

这里还有一个容易踩的点：Linux CI 上可能同时有 Zig 和系统自带的 `/usr/bin/objcopy`。如果先选系统 `objcopy`，它可能不认识 Zig 交叉编译出来的 ARM ELF，构建就会在生成 `.bin` 时失败。因此项目现在的选择顺序是：

```text
EW_FIRMWARE_OBJCOPY 显式指定
  -> zig objcopy
  -> CMAKE_OBJCOPY
```

简单理解：你手动指定的工具最优先；没有手动指定时，既然固件是 Zig 交叉编译出来的，就先用 Zig 自己带的 objcopy。

## 怎么确认产物生成了？

构建固件后看对应 build 目录：

```text
build-fw/embedded_firmware.elf
build-fw/embedded_firmware.bin
build-fw/embedded_firmware.hex
```

真实 USART2 命令 loop 构建对应：

```text
build-fw-real-usart2-command/embedded_firmware.elf
build-fw-real-usart2-command/embedded_firmware.bin
build-fw-real-usart2-command/embedded_firmware.hex
```

## 这还不是烧录

生成 `.bin/.hex` 只是让固件更接近可上板验证。真正验证还需要连接 NUCLEO-F401RE、使用烧录工具写入 flash，并通过调试器或串口观察运行结果。
