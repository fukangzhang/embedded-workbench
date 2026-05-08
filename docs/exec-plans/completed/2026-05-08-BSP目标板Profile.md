# 目标
建立第一版 BSP 目标板 profile，明确后续目标板接入的板卡选择、核心资源和逻辑外设映射，同时保持硬件实现可替换。

## 范围
- 在 `bsp/` 中新增板卡 profile 接口和实现
- 默认推荐目标板为 `NUCLEO-F401RE`
- 保留 `NUCLEO-F103RB` 作为备选 profile
- 描述 MCU 名称、调试器、连接器、控制台 UART、传感器总线和告警输出引脚
- 增加 BSP profile 测试
- 补充目标板选择与 BSP 学习笔记

## 暂不做
- 不引入 STM32 HAL 或 LL
- 不写启动文件和链接脚本
- 不配置真实 GPIO/UART/I2C 寄存器
- 不接入 OpenOCD 或烧录脚本

## 步骤
1. 参考 ST 官方 Nucleo-64 开发板资料
2. 定义 board profile 结构
3. 实现 F401RE 和 F103RB profile
4. 增加 profile 查询和校验测试
5. 补充学习文档和参考链接
6. 运行构建与测试

## 验证方式
- `cmake -S . -B build -G Ninja ...`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`

## 未决问题
- 最终购买或使用哪块开发板，需要结合手头硬件和预算确认
- 具体引脚映射后续应以对应板卡 user manual 和 CubeMX 工程为准

## 进度记录
- 2026-05-08：创建 BSP 目标板 profile 计划
- 2026-05-08：查阅 ST 官方 NUCLEO-F401RE 和 NUCLEO-F103RB 资料，确认两者均为 Nucleo-64、支持 Arduino/ST morpho 扩展并集成 ST-LINK
- 2026-05-08：新增 BSP board profile 接口、F401RE/F103RB profile 和校验测试
- 2026-05-08：补充 BSP 目标板 profile 学习笔记和 ST 官方参考链接

## 验证记录
- `cmake -S . -B build -G Ninja ...`：通过
- `cmake --build build`：通过
- `ctest --test-dir build --output-on-failure`：通过，`9/9` 测试通过

## 完成记录
- 2026-05-08：BSP 目标板 profile 已完成，下一步可进入目标板工具链/交叉编译预研。
