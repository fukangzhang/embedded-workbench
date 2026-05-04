# Embedded Workbench

一个面向嵌入式开发的通用项目骨架，适合后续扩展为 STM32、ESP32、GD32、MSP430、裸机 MCU 或 RTOS 项目。

## 目录结构

- `app/`: 应用层代码
- `bsp/`: 板级支持包
- `docs/`: 设计说明、协议文档、调试记录
- `drivers/`: 外设驱动与中间层
- `firmware/`: 固件构建输出或第三方固件资源
- `hardware/`: 原理图、引脚定义、硬件说明
- `scripts/`: 烧录、打包、辅助脚本
- `tests/`: 单元测试或仿真测试
- `tools/`: 工具链配置、OpenOCD、JLink 等辅助文件

## 建议用法

1. 先在 `docs/` 里补充目标芯片、开发板和工具链信息。
2. 把芯片相关启动文件、链接脚本和 `HAL` 或 `SDK` 放到对应目录。
3. 根据你的平台选择 `CMake`、`Makefile` 或厂商 IDE 工程作为主构建方式。

## 后续可扩展

- `FreeRTOS` 或其他 RTOS
- `bootloader`
- `protocols/` 目录，用于串口、CAN、Modbus 等协议栈
- `third_party/` 目录，用于外部依赖
