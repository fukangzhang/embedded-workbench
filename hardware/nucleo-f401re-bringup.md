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

## 本次还不验证什么

这份清单不证明：

- FreeRTOS 任务已经在板上稳定调度。
- 串口命令已经能通过 ST-LINK VCP 交互。
- 真实传感器已经接入。
- 蜂鸣器和执行器电路已经可用。

这些应该拆成后续独立 bring-up 任务，每个任务都留下观察结果和失败记录。

## 板上验证记录

第一次实际烧录后，把结果追加在这里：

| 日期 | 固件提交 | 构建目录 | 烧录工具 | 观察结果 | 结论 |
| --- | --- | --- | --- | --- | --- |
| 未执行 | - | - | - | - | 待板上验证 |
