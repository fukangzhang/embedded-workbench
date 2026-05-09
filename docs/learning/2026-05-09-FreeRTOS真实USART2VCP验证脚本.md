# FreeRTOS 真实 USART2 VCP 验证脚本

## 这一步做了什么？

新增脚本：

```text
scripts/verify_freertos_usart2_vcp.ps1
```

它用于真实板卡连接后，通过 ST-LINK VCP 串口发送固定命令序列，并打印响应。

默认命令是：

```text
STATUS?
CONFIG?
SAMPLE 360 600 250 20
STATUS?
```

## 为什么需要脚本？

手动打开串口终端当然也可以，但脚本有三个好处：

- 每次发送的命令一样，验证结果更容易对比。
- 可以保存 transcript，后续写验证记录时有证据。
- 对初学者更友好，不容易忘记串口参数或命令顺序。

## 怎么先不碰硬件地检查？

列出 Windows 当前看到的串口：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 -ListPorts
```

只看脚本会做什么，不打开串口：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 -DryRun
```

## 真实运行方式

假设 ST-LINK VCP 是 `COM5`：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 -Port COM5
```

保存响应记录：

```powershell
.\scripts\verify_freertos_usart2_vcp.ps1 `
  -Port COM5 `
  -LogPath hardware\logs\freertos-usart2-vcp-2026-05-09.txt
```

## 运行前必须确认什么？

先构建并烧录这个固件：

```text
build-fw-scheduler-real-usart2-io/embedded_firmware.elf
```

它对应：

```text
FreeRTOS scheduler
  + USART2 command reader
  + USART2 response writer
```

如果烧的是裸机 `build-fw-real-usart2-command/embedded_firmware.elf`，脚本仍可能收到响应，但验证的不是 FreeRTOS 任务路径。

## 这一步还不证明什么？

脚本本身只提供验证入口。只有在真实板卡上运行并把 transcript 写回 `hardware/nucleo-f401re-bringup.md` 后，才算完成板上串口命令闭环验证。
