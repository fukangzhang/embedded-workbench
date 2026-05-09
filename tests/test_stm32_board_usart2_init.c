#include <stdbool.h>
#include <stdint.h>

#include "embedded_workbench/stm32_board_usart2_init.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_u32(uint32_t actual, uint32_t expected)
{
    return actual == expected ? 0 : 1;
}

static uint32_t with_two_bit_field(uint32_t base, unsigned int pin, uint32_t value)
{
    uint32_t shift = pin * 2u;
    uint32_t mask = 3u << shift;

    return (base & ~mask) | ((value & 3u) << shift);
}

static uint32_t with_four_bit_field(uint32_t base, unsigned int field, uint32_t value)
{
    uint32_t shift = field * 4u;
    uint32_t mask = 15u << shift;

    return (base & ~mask) | ((value & 15u) << shift);
}

static int test_init_configures_gpio_and_usart(void)
{
    /* 这组变量都是主机上的“假寄存器”。
     * 测试不会访问真实 STM32 地址，而是让 driver 写这些普通内存，再检查位是否正确。 */
    volatile uint32_t ahb1enr = 0u;
    volatile uint32_t apb1enr = 0u;
    stm32_gpio_registers_t gpioa = {0u};
    stm32_usart_registers_t usart2 = {0u};

    /* 表格把名字映射到 bit 或寄存器块。真实固件会使用 bindings 提供的地址；
     * 主机测试用本地变量替代，验证同一套逻辑不会依赖硬件才能跑。 */
    stm32_rcc_gpio_clock_port_t gpio_clocks[] = {{"PA", 0u}};
    stm32_gpio_config_port_t gpio_ports[] = {{"PA", &gpioa}};
    stm32_rcc_usart_clock_peripheral_t usart_clocks[] = {{"USART2", 17u}};
    stm32_rcc_gpio_clock_context_t gpio_clock_context;
    stm32_gpio_config_context_t gpio_context;
    stm32_rcc_usart_clock_context_t usart_clock_context;
    stm32_usart_config_t usart_config = stm32_usart_config_default(16000000u, 9600u);

    if (!stm32_rcc_gpio_clock_init(&gpio_clock_context, &ahb1enr, gpio_clocks, 1u) ||
        !stm32_gpio_config_init(&gpio_context, gpio_ports, 1u) ||
        !stm32_rcc_usart_clock_init(&usart_clock_context, &apb1enr, usart_clocks, 1u)) {
        return 1;
    }

    if (expect_true(stm32_board_usart2_init(
            &gpio_clock_context,
            &gpio_context,
            &usart_clock_context,
            &usart2,
            &usart_config)) != 0) {
        return 2;
    }

    /* GPIOA clock bit0、USART2 APB1 bit17 都被打开。 */
    if (expect_u32(ahb1enr, 1u << 0u) != 0 ||
        expect_u32(apb1enr, 1u << 17u) != 0) {
        return 3;
    }

    /* PA2/PA3 都进入 alternate mode，AFRL 对应字段写 AF7。 */
    if (expect_u32(gpioa.moder, with_two_bit_field(with_two_bit_field(0u, 2u, 2u), 3u, 2u)) != 0 ||
        expect_u32(gpioa.ospeedr, with_two_bit_field(with_two_bit_field(0u, 2u, 3u), 3u, 3u)) != 0 ||
        expect_u32(gpioa.pupdr, with_two_bit_field(with_two_bit_field(0u, 2u, 1u), 3u, 1u)) != 0 ||
        expect_u32(gpioa.afrl, with_four_bit_field(with_four_bit_field(0u, 2u, 7u), 3u, 7u)) != 0) {
        return 4;
    }

    /* USART2 配置为 9600 8N1，并打开 UE/TE/RE。 */
    if (expect_u32(usart2.brr, 1667u) != 0 ||
        expect_u32(usart2.cr1 & ((1u << 13u) | (1u << 3u) | (1u << 2u)),
            (1u << 13u) | (1u << 3u) | (1u << 2u)) != 0) {
        return 5;
    }

    return 0;
}

static int test_invalid_arguments_fail(void)
{
    /* 初始化函数是板级入口，边界处要先挡住空指针。
     * 否则后续 driver 可能在解引用 context/registers 时让程序崩掉。 */
    volatile uint32_t ahb1enr = 0u;
    volatile uint32_t apb1enr = 0u;
    stm32_gpio_registers_t gpioa = {0u};
    stm32_usart_registers_t usart2 = {0u};
    stm32_rcc_gpio_clock_port_t gpio_clocks[] = {{"PA", 0u}};
    stm32_gpio_config_port_t gpio_ports[] = {{"PA", &gpioa}};
    stm32_rcc_usart_clock_peripheral_t usart_clocks[] = {{"USART2", 17u}};
    stm32_rcc_gpio_clock_context_t gpio_clock_context;
    stm32_gpio_config_context_t gpio_context;
    stm32_rcc_usart_clock_context_t usart_clock_context;
    stm32_usart_config_t usart_config = stm32_usart_config_default(16000000u, 9600u);

    if (!stm32_rcc_gpio_clock_init(&gpio_clock_context, &ahb1enr, gpio_clocks, 1u) ||
        !stm32_gpio_config_init(&gpio_context, gpio_ports, 1u) ||
        !stm32_rcc_usart_clock_init(&usart_clock_context, &apb1enr, usart_clocks, 1u)) {
        return 1;
    }

    if (expect_false(stm32_board_usart2_init(0, &gpio_context, &usart_clock_context, &usart2, &usart_config)) != 0 ||
        expect_false(stm32_board_usart2_init(&gpio_clock_context, 0, &usart_clock_context, &usart2, &usart_config)) != 0 ||
        expect_false(stm32_board_usart2_init(&gpio_clock_context, &gpio_context, 0, &usart2, &usart_config)) != 0 ||
        expect_false(stm32_board_usart2_init(&gpio_clock_context, &gpio_context, &usart_clock_context, 0, &usart_config)) != 0 ||
        expect_false(stm32_board_usart2_init(&gpio_clock_context, &gpio_context, &usart_clock_context, &usart2, 0)) != 0) {
        return 2;
    }

    return 0;
}

static int test_missing_usart_clock_fails_before_usart_config(void)
{
    /* 这里故意只提供 USART3，不提供 USART2。
     * 目标是证明“找不到 USART2 时钟绑定”会让流程失败，并且不会继续写 USART2 BRR/CR1。 */
    volatile uint32_t ahb1enr = 0u;
    volatile uint32_t apb1enr = 0u;
    stm32_gpio_registers_t gpioa = {0u};
    stm32_usart_registers_t usart2 = {0u};
    stm32_rcc_gpio_clock_port_t gpio_clocks[] = {{"PA", 0u}};
    stm32_gpio_config_port_t gpio_ports[] = {{"PA", &gpioa}};
    stm32_rcc_usart_clock_peripheral_t usart_clocks[] = {{"USART3", 18u}};
    stm32_rcc_gpio_clock_context_t gpio_clock_context;
    stm32_gpio_config_context_t gpio_context;
    stm32_rcc_usart_clock_context_t usart_clock_context;
    stm32_usart_config_t usart_config = stm32_usart_config_default(16000000u, 9600u);

    if (!stm32_rcc_gpio_clock_init(&gpio_clock_context, &ahb1enr, gpio_clocks, 1u) ||
        !stm32_gpio_config_init(&gpio_context, gpio_ports, 1u) ||
        !stm32_rcc_usart_clock_init(&usart_clock_context, &apb1enr, usart_clocks, 1u)) {
        return 1;
    }

    if (expect_false(stm32_board_usart2_init(
            &gpio_clock_context,
            &gpio_context,
            &usart_clock_context,
            &usart2,
            &usart_config)) != 0 ||
        expect_u32(apb1enr, 0u) != 0 ||
        expect_u32(usart2.brr, 0u) != 0) {
        return 2;
    }

    return 0;
}

int main(void)
{
    if (test_init_configures_gpio_and_usart() != 0) {
        return 1;
    }
    if (test_invalid_arguments_fail() != 0) {
        return 2;
    }
    if (test_missing_usart_clock_fails_before_usart_config() != 0) {
        return 3;
    }

    return 0;
}
