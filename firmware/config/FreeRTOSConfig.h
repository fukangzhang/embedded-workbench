#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

/* FreeRTOS 用这个值计算 tick 和定时器节拍；真实值由启动/系统时钟代码提供。 */
extern uint32_t SystemCoreClock;

/* 使用抢占式调度：高优先级任务就绪时可以打断低优先级任务。 */
#define configUSE_PREEMPTION 1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE 0
#define configCPU_CLOCK_HZ (SystemCoreClock)
/* 1 ms 一个 tick，方便把任务周期写成毫秒级。 */
#define configTICK_RATE_HZ ((TickType_t)1000)
/* 任务模型里最高优先级目前小于 5，这里给 FreeRTOS 留出 0..4 共 5 档。 */
#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE ((uint16_t)128)
/* heap_4.c 会从这块堆里分配 task/queue/timer 等 RTOS 对象。 */
#define configTOTAL_HEAP_SIZE ((size_t)(16 * 1024))
#define configMAX_TASK_NAME_LEN 16
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 1
#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 0
#define configUSE_COUNTING_SEMAPHORES 1
#define configQUEUE_REGISTRY_SIZE 8
#define configUSE_QUEUE_SETS 0
#define configUSE_TIME_SLICING 1
#define configUSE_NEWLIB_REENTRANT 0
#define configENABLE_BACKWARD_COMPATIBILITY 0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0
#define configSTACK_DEPTH_TYPE uint16_t
#define configMESSAGE_BUFFER_LENGTH_TYPE size_t

/* 当前骨架用动态创建 task/queue，后续若要更强确定性可再切到静态分配。 */
#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* 早期骨架先关闭 hook，等需要诊断栈溢出/分配失败时再单独接入处理函数。 */
#define configCHECK_FOR_STACK_OVERFLOW 0
#define configUSE_MALLOC_FAILED_HOOK 0
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0

/* 软件定时器打开后，FreeRTOS 会额外创建 timer service task。 */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY 3
#define configTIMER_QUEUE_LENGTH 8
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2)

/* Cortex-M 中断优先级配置：数值越小优先级越高，能调用 FreeRTOS API 的中断要受限制。 */
#define configPRIO_BITS 4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* 断言失败时关中断并停在死循环，方便调试器定位第一现场。 */
#define configASSERT(x) \
    do {                \
        if ((x) == 0) { \
            taskDISABLE_INTERRUPTS(); \
            for (;;) {  \
            }           \
        }               \
    } while (0)

/* INCLUDE_* 控制哪些 FreeRTOS API 会被编译进来；先保留常用调度/延时/诊断接口。 */
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xResumeFromISR 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1

/* 把 FreeRTOS port 需要的异常处理函数名映射到启动文件里的 Cortex-M 向量名。 */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif
