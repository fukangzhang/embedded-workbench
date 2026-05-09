#include <stdint.h>

/* 这些符号由 linker script 定义，不是在 C 文件里分配的普通变量。
 * startup 代码用它们知道栈顶、.data 源地址、.data/.bss 的内存范围。 */
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

int main(void);
void Reset_Handler(void);
void Default_Handler(void);

/* weak alias 表示：如果别的文件实现了同名中断处理函数，就用别的文件的；
 * 如果没有实现，就退回 Default_Handler。早期固件骨架因此可以先不写所有中断。 */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

__attribute__((section(".isr_vector")))
void (*const vector_table[])(void) = {
    /* Cortex-M 启动时会先从向量表第 0 项取初始栈指针。 */
    (void (*)(void))(&_estack),
    /* 第 1 项是复位入口。上电或复位后 CPU 会跳到 Reset_Handler。 */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,
};

void Reset_Handler(void)
{
    uint32_t *source = &_sidata;
    uint32_t *destination = &_sdata;

    /* 把 .data 从 Flash 中的加载地址复制到 RAM 中的运行地址。
     * 这一步让带初始值的全局/static 变量在 main 前拥有正确值。 */
    while (destination < &_edata) {
        *destination = *source;
        destination++;
        source++;
    }

    destination = &_sbss;
    /* 清零 .bss，让未显式初始化的全局/static 变量按 C 语言规则从 0 开始。 */
    while (destination < &_ebss) {
        *destination = 0u;
        destination++;
    }

    /* C 运行环境准备好后进入固件 main。当前 main 返回也不应回到未知位置。 */
    (void)main();

    while (1) {
    }
}

void Default_Handler(void)
{
    /* 没有专门处理函数的异常会停在这里，方便调试器定位“意外中断/异常”。 */
    while (1) {
    }
}
