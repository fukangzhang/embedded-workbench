# FreeRTOS 真实 USART2 I/O 板上验证入口

## 这一步做了什么？

新增本地构建脚本：

```text
scripts/build_freertos_usart2_io_firmware.ps1
```

它专门生成后续板上验证要用的固件组合：

```text
FreeRTOS scheduler
  + 真实 USART2 command reader
  + 真实 USART2 response writer
```

对应 CMake 开关是：

```text
EW_USE_FREERTOS=ON
EW_FREERTOS_START_SCHEDULER=ON
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER=ON
EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_RESPONSE_WRITER=ON
```

## 为什么需要单独脚本？

这个构建组合的参数很多，手敲很容易漏掉某个开关。

如果只打开 reader/writer，但没有打开 scheduler，固件只能证明 FreeRTOS port 能编译链接，不能进入真实任务调度。

如果只打开 scheduler，但没有打开 reader/writer，任务能调度，但串口命令不会从真实 USART2 进出。

所以板上验证要用一个明确入口，避免把几种固件混着烧。

## 怎么使用？

先 dry-run，看脚本会执行什么：

```powershell
.\scripts\build_freertos_usart2_io_firmware.ps1 -DryRun
```

确认路径没问题后构建：

```powershell
.\scripts\build_freertos_usart2_io_firmware.ps1
```

默认输出：

```text
build-fw-scheduler-real-usart2-io/embedded_firmware.elf
build-fw-scheduler-real-usart2-io/embedded_firmware.bin
build-fw-scheduler-real-usart2-io/embedded_firmware.hex
```

## 怎么烧录？

构建完成后，可以用已有 OpenOCD 脚本指定镜像：

```powershell
.\scripts\flash_nucleo_f401re_openocd.ps1 `
  -ImagePath build-fw-scheduler-real-usart2-io\embedded_firmware.elf
```

第一次仍然先 dry-run：

```powershell
.\scripts\flash_nucleo_f401re_openocd.ps1 `
  -ImagePath build-fw-scheduler-real-usart2-io\embedded_firmware.elf `
  -DryRun
```

## 上板后应该看什么？

串口参数：

```text
9600 8N1
```

先发：

```text
STATUS?
```

再发：

```text
CONFIG?
```

然后可以手工注入样本：

```text
SAMPLE 360 600 250 20
STATUS?
```

如果链路跑通，应能看到响应文本从 ST-LINK VCP 返回。

## CI 验证了什么？

CI 新增 `build-fw-scheduler-real-usart2-io` 构建组合。

它证明这个待烧录固件能编译链接，但不证明真实板上串口已经收发成功。真正的结论仍然要写进 `hardware/nucleo-f401re-bringup.md` 的板上验证记录。
