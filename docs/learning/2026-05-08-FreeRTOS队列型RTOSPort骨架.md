# FreeRTOS 队列型 RTOS Port 骨架

## 这一阶段在做什么？
上一阶段已经能把 FreeRTOS-Kernel 链进固件，但应用层还没有真正调用 FreeRTOS 的通信 API。

这一阶段新增 `rtos_port_freertos`：

- 用 `xQueueCreate` 创建队列
- 用 `xQueueSend` 发送传感器样本和命令
- 用 `xQueueReceive` 接收响应消息
- 在 firmware FreeRTOS 构建下做一次最小队列收发自检

这说明现有 `rtos_port` 抽象已经开始落到真实 FreeRTOS queue 上。

## FreeRTOS queue 是什么？
queue 可以理解为任务之间传数据的管道。

例如后续会有这样的数据流：

- 传感器任务采集 `sensor_sample_t`
- 把样本发送到 `sensor_sample_queue`
- 处理任务从队列里取出样本，判断告警状态
- 通信任务把状态格式化成响应消息

当前 `sensor_acquire_task` 已经能从可选 `sensor_source` 读取 sample，再用 `xQueueSend` 放入 `sensor_sample_queue`。如果 source 为空，它仍然只保持周期任务骨架，方便早期固件构建验证。

FreeRTOS queue 默认是“按值复制”。也就是说，`xQueueSend` 会把你传入的数据复制进队列内部存储，而不是只保存一个指针。

这对嵌入式项目很重要，因为任务之间如果只传指针，很容易出现生命周期和并发问题。

## 为什么先做队列，不直接启动调度器？
`vTaskStartScheduler()` 正常情况下不会返回。它一旦启动，CPU 就进入 FreeRTOS 的任务调度世界。

但当前还没有真实任务函数，也没有板上中断和 tick 验证。如果现在就启动 scheduler，问题会混在一起：

- 是队列创建错了？
- 是任务栈不够？
- 是 tick 中断没接对？
- 是 PendSV/SVC handler 没接对？
- 是 Zig 临时工具链和真实 ARM GNU Toolchain 行为不同？

所以这一阶段只验证队列 API 能接入，调度器启动留到下一步。

## rtos_port_freertos 做了什么？
新增的上下文结构保存三个 FreeRTOS queue handle：

```c
typedef struct {
    QueueHandle_t sensor_sample_queue;
    QueueHandle_t command_queue;
    QueueHandle_t response_queue;
} freertos_rtos_port_context_t;
```

`freertos_rtos_port_init()` 负责创建这些队列，并把操作表填进通用的 `rtos_port_t`。

这样应用层仍然调用：

```c
rtos_port_send_sensor_sample(...)
rtos_port_send_command(...)
rtos_port_receive_response(...)
```

底层才知道这些调用实际落到了 FreeRTOS queue。

## 为什么补了 __aeabi_memclr？
在裸机/freestanding 编译里，编译器有时会把结构体清零优化成运行时辅助函数，例如：

```c
__aeabi_memclr
```

普通桌面程序由系统 C 运行时提供这些符号；裸机固件没有完整 C 运行时，所以需要我们在最小 libc 里补上。

这类符号不是业务逻辑，而是“让编译器生成的裸机代码能链接起来”的底层胶水。

## 当前验证说明
当前验证覆盖三条路径：

- host 测试仍然通过，说明 FreeRTOS 实现没有污染主机仿真
- 普通 firmware ELF 仍然能构建，说明无 RTOS 路径没坏
- FreeRTOS firmware ELF clean-first 构建通过，说明 queue-backed port 真实参与编译和链接

## 后续还缺什么？
下一步可以进入真实任务骨架：

- 创建任务函数
- 用 `xTaskCreate` 创建任务
- 让任务阻塞等待队列消息
- 明确 `rtos_port_start()` 和 `vTaskStartScheduler()` 的关系
- 做板上 tick / SVC / PendSV 验证前的准备

## 参考资料
- FreeRTOS-Kernel `queue.h`
- FreeRTOS-Kernel `task.h`
- FreeRTOS queue API reference: https://www.freertos.org/a00118.html
