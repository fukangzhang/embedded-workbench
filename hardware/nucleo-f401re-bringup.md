# NUCLEO-F401RE Bring-up Checklist

## 目标

这份清单用于第一次把当前固件放到 NUCLEO-F401RE 上验证。它不是泛泛的硬件介绍，而是回答：

- 当前代码认为哪些引脚是告警输出？
- 下载前要构建哪个固件？
- 上板后应该观察什么？
- 失败时先查哪一层？

## 当前板卡事实

目标板：

- Board：NUCLEO-F401RE
- MCU：STM32F401RE
- Debug probe：板载 ST-LINK/V2-1
- 当前默认 profile：`BOARD_ID_NUCLEO_F401RE`

当前代码里的告警输出映射来自 `bsp/src/board_profile.c`：

| 功能 | STM32 pin | 代码用途 | 当前状态 |
| --- | --- | --- | --- |
| `alarm_led` | `PA5` | 告警指示灯 | 对应 Nucleo 板载 LD2 / Arduino D13 |
| `alarm_buzzer` | `PB6` | 外部蜂鸣器占位 | 对应 Arduino D10，需要外接电路 |
| `alarm_actuator` | `PB7` | 外部执行器使能占位 | 当前只是项目占位，需要确认外接位置 |

官方资料依据：

- ST `UM1724 STM32 Nucleo-64 boards (MB1136)` 的 NUCLEO-F401RE/F411RE Arduino connector 表显示：`D13 -> PA5`、`D10 -> PB6`、`D1 -> PA2 USART2_TX`、`D0 -> PA3 USART2_RX`。
- 官方手册链接：<https://www.st.com/resource/en/user_manual/dm00105823-nucleo-f401re-user-manual-stmicroelectronics.pdf>

## 下载前检查

先确认代码处在 `main` 或目标 PR 分支，并且本地没有未提交改动：

```powershell
git status --short --branch
```

构建普通固件：

```powershell
& 'C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\cmake.exe' --build build-fw
```

构建开启真实 STM32 GPIO 初始化路径的固件：

```powershell
$env:ZIG_EXE='C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Lib\site-packages\ziglang\zig.exe'
$env:ZIG_AR='C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\zig-ar.cmd'
$env:ZIG_RANLIB='C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\zig-ranlib.cmd'
& 'C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\cmake.exe' -S . -B build-fw-real-gpio -G Ninja `
  -D CMAKE_MAKE_PROGRAM='C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\ninja.exe' `
  -D CMAKE_TOOLCHAIN_FILE=cmake/toolchains/zig-arm-none-eabi.cmake `
  -D EW_BUILD_HOST_TOOLS=OFF `
  -D EW_BUILD_TESTS=OFF `
  -D EW_BUILD_FIRMWARE=ON `
  -D EW_FIRMWARE_USE_REAL_STM32_GPIO_INIT=ON
& 'C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\cmake.exe' --build build-fw-real-gpio
```

本轮真实 GPIO 初始化验证应使用：

```text
build-fw-real-gpio/embedded_firmware.elf
build-fw-real-gpio/embedded_firmware.bin
build-fw-real-gpio/embedded_firmware.hex
```

构建开启真实 STM32 USART2 命令轮询路径的固件：

```powershell
& 'C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\cmake.exe' -S . -B build-fw-real-usart2-command -G Ninja `
  -D CMAKE_MAKE_PROGRAM='C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\ninja.exe' `
  -D CMAKE_TOOLCHAIN_FILE=cmake/toolchains/zig-arm-none-eabi.cmake `
  -D ZIG_EXE='C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Lib\site-packages\ziglang\zig.exe' `
  -D ZIG_AR='C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\zig-ar.cmd' `
  -D ZIG_RANLIB='C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\zig-ranlib.cmd' `
  -D EW_BUILD_HOST_TOOLS=OFF `
  -D EW_BUILD_TESTS=OFF `
  -D EW_BUILD_FIRMWARE=ON `
  -D EW_FIRMWARE_USE_REAL_STM32_USART2_COMMAND_LOOP=ON
& 'C:\Users\fukan\.codex\venvs\embedded-workbench-toolchain\Scripts\cmake.exe' --build build-fw-real-usart2-command
```

真实 USART2 命令验证应使用：

```text
build-fw-real-usart2-command/embedded_firmware.elf
build-fw-real-usart2-command/embedded_firmware.bin
build-fw-real-usart2-command/embedded_firmware.hex
```

构建开启 FreeRTOS scheduler、真实 USART2 command reader 和真实 USART2 response writer 的固件：

```powershell
.\scripts\build_freertos_usart2_io_firmware.ps1
```

第一次只想确认脚本会执行哪些命令，可以先 dry-run：

```powershell
.\scripts\build_freertos_usart2_io_firmware.ps1 -DryRun
```

FreeRTOS 真实 USART2 I/O 板上验证应使用：

```text
build-fw-scheduler-real-usart2-io/embedded_firmware.elf
build-fw-scheduler-real-usart2-io/embedded_firmware.bin
build-fw-scheduler-real-usart2-io/embedded_firmware.hex
```

`ELF` 更适合调试器，`BIN/HEX` 更适合烧录工具。具体用哪个取决于后续选择 STM32CubeProgrammer、OpenOCD 还是其他 ST-LINK 工具。

## OpenOCD 烧录入口

当前仓库提供了一个 OpenOCD 辅助脚本。第一次使用先 dry-run，只看命令，不连接板子：

```powershell
.\scripts\flash_nucleo_f401re_openocd.ps1 -DryRun
```

默认镜像是：

```text
build-fw-real-usart2-command/embedded_firmware.elf
```

真实烧录时，先确认 OpenOCD 已安装、NUCLEO-F401RE 已通过 ST-LINK USB 口连接，再运行：

```powershell
.\scripts\flash_nucleo_f401re_openocd.ps1
```

如果要烧录 `.bin`，脚本会自动补 STM32F401RE 内部 flash 起始地址 `0x08000000`：

```powershell
.\scripts\flash_nucleo_f401re_openocd.ps1 -ImagePath build-fw-real-usart2-command\embedded_firmware.bin
```

如果要烧录 FreeRTOS 真实 USART2 I/O 固件，显式指定新构建目录：

```powershell
.\scripts\flash_nucleo_f401re_openocd.ps1 -ImagePath build-fw-scheduler-real-usart2-io\embedded_firmware.elf
```

## ST-LINK VCP 串口验证入口

烧录 FreeRTOS 真实 USART2 I/O 固件后，先查看 Windows 识别到的串口：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 -ListPorts
```

只看验证脚本会发送哪些命令，不打开串口：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 -DryRun
```

假设 ST-LINK VCP 是 `COM5`，真实检查命令为：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 -Port COM5
```

如果要保留串口响应记录：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 `
  -Port COM5 `
  -LogPath hardware\logs\freertos-usart2-vcp-2026-05-09.txt
```

脚本默认发送：

```text
STATUS?
CONFIG?
SAMPLE 360 600 250 20
STATUS?
```

脚本默认还会检查 transcript 是否包含：

```text
OK result=ok
CONFIG temp_warn=
STATUS state=warning
```

如果只是想保存串口原始响应，不做默认期望检查，可以加：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 -Port COM5 -NoExpect
```

如果本次验证目标不同，可以用 `-Expect` 传入自定义期望片段。

如果已经保存过 transcript，也可以离线复查，不重新打开串口：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 `
  -ReplayLogPath hardware\logs\freertos-usart2-vcp-2026-05-09.txt
```

如果 `openocd` 不在 PATH 中，可以显式传入可执行文件路径：

```powershell
.\scripts\flash_nucleo_f401re_openocd.ps1 -OpenOcd 'C:\tools\openocd\bin\openocd.exe'
```

## 烧录前硬件检查

1. 用 USB 连接 NUCLEO-F401RE 的 ST-LINK USB 口。
2. 确认 `LD3/PWR` 电源灯正常。
3. 暂时不要外接蜂鸣器和执行器，先只观察板载 `LD2`。
4. 外接蜂鸣器或执行器前，先确认驱动电路、电流限制和电源地线；不要直接用 GPIO 驱动大电流负载。

## 第一次观察目标

当前 `firmware/src/main.c` 是自检入口，不是完整应用循环。打开真实 GPIO 初始化开关后，它会额外执行：

```text
stm32f401re_gpio_bindings
  -> stm32_rcc_gpio_clock
  -> stm32_gpio_config
  -> stm32_board_gpio_init
```

第一次板上验证先看这些：

- 固件是否能下载到目标板。
- 下载后板子是否没有反复复位。
- 如果调试器能读变量，检查 `firmware_self_check`：
  - `1`：当前自检链路通过。
  - `-1`：普通自检或真实 GPIO 初始化失败。
  - `-2`：FreeRTOS 调度器启动路径返回，当前不应作为默认 bring-up 目标。
- 如果使用调试器单步，确认 `firmware_stm32f401re_real_gpio_init` 返回 `true`。

## 失败定位顺序

按这个顺序查，别一上来怀疑所有东西：

1. 固件无法构建

   先看 `build-fw-real-gpio` 配置是否打开 `EW_FIRMWARE_USE_REAL_STM32_GPIO_INIT`，以及 Zig 路径是否正确。

2. 固件无法下载

   先检查 ST-LINK 是否识别、USB 线是否支持数据、板子供电是否正常。

3. 下载后进不了 `main`

   先看启动文件和 linker script，当前入口是 `Reset_Handler -> main`。

4. `firmware_self_check == -1`

   先单步 `firmware_stm32_gpio_init_self_check`，确认模拟寄存器链路仍然通过；再单步 `firmware_stm32f401re_real_gpio_init`，确认 RCC/GPIO 初始化返回值。

5. `PA5` 没有预期电平

   先确认 RCC GPIOA 时钟已打开，再看 GPIOA `MODER` 中 pin 5 是否为输出模式。

6. 外部蜂鸣器/执行器不动作

   先确认外接电路，不要直接把失败归因于软件。`PB6/PB7` 当前仍是占位输出，真正接线后应更新本文件和 `board_profile`。

7. 串口没有响应

   裸机命令 loop 先确认使用的是 `build-fw-real-usart2-command/embedded_firmware.elf`。FreeRTOS 任务闭环先确认使用的是 `build-fw-scheduler-real-usart2-io/embedded_firmware.elf`。串口参数为 `9600 8N1`，并且终端发送了 `STATUS?` 加换行。再检查 PA2/PA3 是否走 ST-LINK VCP，对应 `USART2_TX/USART2_RX`。

8. 想在没有传感器时触发 warning

   可以发送 `SAMPLE 360 600 250 20` 加换行，再发送 `STATUS?`。如果串口链路正常，应看到 `STATUS state=warning` 和 `indicator=slow_blink`。这只是手工注入样本，不代表真实传感器已经接入。

## 本次还不验证什么

这份清单本身不证明：

- 真实传感器已经接入。
- 蜂鸣器和执行器电路已经可用。

FreeRTOS 任务调度和 USART2 命令 I/O 现在已经有待烧录固件入口，但还需要真实板卡观察后，才能把结论写入下面的验证记录。

## 板上验证记录

第一次实际烧录后，把结果追加在这里：

| 日期 | 固件提交 | 构建目录 | 烧录工具 | 观察结果 | 结论 |
| --- | --- | --- | --- | --- | --- |
| 未执行 | - | - | - | - | 待板上验证 |
