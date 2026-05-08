#include "embedded_workbench/alarm_output.h"
#include "embedded_workbench/alarm_output_digital_sink.h"
#include "embedded_workbench/alarm_state.h"
#include "embedded_workbench/board_digital_output.h"
#include "embedded_workbench/board_profile.h"
#include "embedded_workbench/command_handler.h"
#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/response_format.h"
#include "embedded_workbench/rtos_port.h"
#include "embedded_workbench/rtos_task_model.h"
#include "embedded_workbench/sensor_sample.h"
#include "embedded_workbench/stm32_board_gpio_init.h"

#if defined(EW_FIRMWARE_USE_REAL_STM32_GPIO_INIT)
#include "embedded_workbench/stm32f401re_gpio_bindings.h"
#endif

#if defined(EW_FIRMWARE_USE_FREERTOS)
#include "embedded_workbench/rtos_port_freertos.h"
#endif

static volatile alarm_state_t firmware_last_state = ALARM_STATE_NORMAL;
static volatile int firmware_self_check = 0;
uint32_t SystemCoreClock = 16000000u;

/* 这些 static 对象模拟“固件全局资源”：
 * 裸机环境没有操作系统进程的生命周期，很多驱动上下文会在整个固件运行期间一直存在。 */
static board_digital_output_context_t firmware_board_output_context = {0};
static digital_output_controller_t firmware_digital_output = {0};
static alarm_output_digital_sink_context_t firmware_alarm_output_digital_context = {0};
static alarm_output_sink_t firmware_alarm_output_sink = {0};

/* 下面这组不是 STM32 真实地址，而是固件自检用的模拟寄存器。
 * 真实地址绑定在 stm32f401re_gpio_bindings 模块里，当前 main 仍然保持可构建、可链接的安全自检。 */
static volatile uint32_t firmware_rcc_ahb1enr = 0u;
static stm32_gpio_registers_t firmware_gpioa_registers = {0};
static stm32_gpio_registers_t firmware_gpiob_registers = {0};

static bool firmware_alarm_output_self_check(void)
{
    const board_profile_t *profile = board_profile_default();
    alarm_output_command_t command;
    digital_output_level_t led_level = DIGITAL_OUTPUT_LEVEL_LOW;
    digital_output_level_t buzzer_level = DIGITAL_OUTPUT_LEVEL_LOW;
    digital_output_level_t actuator_level = DIGITAL_OUTPUT_LEVEL_LOW;

    /* 固件早期没有真实外设流程时，用主机同款数字输出链路做一次端到端自检：
     * 状态 -> 输出命令 -> sink -> board 数字输出，最终检查三个输出是否都被拉高。 */
    return board_digital_output_init(&firmware_digital_output, &firmware_board_output_context, profile) &&
           alarm_output_digital_sink_init(
               &firmware_alarm_output_sink,
               &firmware_alarm_output_digital_context,
               &firmware_digital_output,
               profile) &&
           alarm_output_command_for_state(ALARM_STATE_ALARM, &command) &&
           alarm_output_sink_apply(&firmware_alarm_output_sink, &command) &&
           board_digital_output_get_level(&firmware_board_output_context, &profile->alarm_led, &led_level) &&
           board_digital_output_get_level(&firmware_board_output_context, &profile->alarm_buzzer, &buzzer_level) &&
           board_digital_output_get_level(&firmware_board_output_context, &profile->alarm_actuator, &actuator_level) &&
           led_level == DIGITAL_OUTPUT_LEVEL_HIGH &&
           buzzer_level == DIGITAL_OUTPUT_LEVEL_HIGH &&
           actuator_level == DIGITAL_OUTPUT_LEVEL_HIGH;
}

static bool gpio_pin_is_output(const stm32_gpio_registers_t *registers, unsigned int pin)
{
    uint32_t mode = 0u;

    if (registers == 0 || pin > 15u) {
        return false;
    }

    /* MODER 每个 pin 占两位，01 表示通用输出模式。 */
    mode = (registers->moder >> (pin * 2u)) & 3u;

    return mode == 1u;
}

static bool firmware_stm32_gpio_init_self_check(void)
{
    const board_profile_t *profile = board_profile_default();
    stm32_rcc_gpio_clock_port_t clock_ports[] = {
        {"PA", 0u},
        {"PB", 1u},
    };
    stm32_gpio_config_port_t gpio_ports[] = {
        {"PA", &firmware_gpioa_registers},
        {"PB", &firmware_gpiob_registers},
    };
    stm32_rcc_gpio_clock_context_t clock_context;
    stm32_gpio_config_context_t gpio_context;

    /* 每次自检前清零模拟寄存器，确保结果来自本次初始化流程。 */
    firmware_rcc_ahb1enr = 0u;
    firmware_gpioa_registers.moder = 0u;
    firmware_gpioa_registers.otyper = 0u;
    firmware_gpioa_registers.ospeedr = 0u;
    firmware_gpioa_registers.pupdr = 0u;
    firmware_gpiob_registers.moder = 0u;
    firmware_gpiob_registers.otyper = 0u;
    firmware_gpiob_registers.ospeedr = 0u;
    firmware_gpiob_registers.pupdr = 0u;

    /* 这里仍然使用模拟寄存器，只验证固件目标能串起：
     * RCC 端口时钟 -> GPIO 输出配置 -> board_profile 告警输出引脚。 */
    return stm32_rcc_gpio_clock_init(&clock_context, &firmware_rcc_ahb1enr, clock_ports, 2u) &&
           stm32_gpio_config_init(&gpio_context, gpio_ports, 2u) &&
           stm32_board_gpio_init_alarm_outputs(profile, &clock_context, &gpio_context, 0) &&
           (firmware_rcc_ahb1enr & ((1u << 0u) | (1u << 1u))) == ((1u << 0u) | (1u << 1u)) &&
           gpio_pin_is_output(&firmware_gpioa_registers, profile->alarm_led.pin) &&
           gpio_pin_is_output(&firmware_gpiob_registers, profile->alarm_buzzer.pin) &&
           gpio_pin_is_output(&firmware_gpiob_registers, profile->alarm_actuator.pin);
}

#if defined(EW_FIRMWARE_USE_REAL_STM32_GPIO_INIT)
static bool firmware_stm32f401re_real_gpio_init(void)
{
    const board_profile_t *profile = board_profile_default();
    stm32_rcc_gpio_clock_context_t clock_context;
    stm32_gpio_config_context_t gpio_context;
    size_t clock_port_count = 0u;
    size_t gpio_port_count = 0u;
    const stm32_rcc_gpio_clock_port_t *clock_ports = stm32f401re_gpio_clock_ports(&clock_port_count);
    const stm32_gpio_config_port_t *gpio_ports = stm32f401re_gpio_config_ports(&gpio_port_count);

    /* 这个路径会解引用 STM32F401RE 的真实内存映射寄存器地址。
     * 它只能在目标板固件中通过显式编译开关启用，主机测试和默认固件构建不会执行。 */
    return stm32_rcc_gpio_clock_init(
               &clock_context,
               stm32f401re_rcc_ahb1enr(),
               clock_ports,
               clock_port_count) &&
           stm32_gpio_config_init(&gpio_context, gpio_ports, gpio_port_count) &&
           stm32_board_gpio_init_alarm_outputs(profile, &clock_context, &gpio_context, 0);
}
#endif

#if defined(EW_FIRMWARE_USE_FREERTOS)
static freertos_rtos_port_context_t firmware_rtos_context = {0};
static rtos_port_t firmware_rtos_port = {0};

static bool firmware_freertos_queue_self_check(const sensor_sample_t *sample, const command_t *command)
{
    rtos_response_message_t queued_response = {{0}};
    rtos_response_message_t received_response = {{0}};

    queued_response.text[0] = 'O';
    queued_response.text[1] = 'K';
    queued_response.text[2] = '\0';

    /* FreeRTOS 自检只验证队列和端口包装能互通，不在这里启动真正的调度器。 */
    return freertos_rtos_port_init(&firmware_rtos_port, &firmware_rtos_context) &&
           rtos_port_start(&firmware_rtos_port) &&
           rtos_port_send_sensor_sample(&firmware_rtos_port, sample) &&
           rtos_port_send_command(&firmware_rtos_port, command) &&
           freertos_rtos_port_send_response(&firmware_rtos_context, &queued_response) &&
           rtos_port_receive_response(&firmware_rtos_port, &received_response) &&
           received_response.text[0] == 'O' &&
           received_response.text[1] == 'K' &&
           received_response.text[2] == '\0';
}
#endif

int main(void)
{
    alarm_config_t config = alarm_config_default();
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    command_t command;
    command_handler_result_t result;
    char response[160];
    bool freertos_ready = true;
    bool real_gpio_ready = true;

    command_init(&command);
    command.type = COMMAND_TYPE_GET_STATUS;

    /* main 当前是固件骨架自检入口：把核心纯逻辑串起来，给链接和启动流程一个可观察结果。 */
    result = command_handler_handle(&command, &config);
    firmware_last_state = alarm_state_update(ALARM_STATE_NORMAL, &config, &sample);

#if defined(EW_FIRMWARE_USE_FREERTOS)
    freertos_ready = firmware_freertos_queue_self_check(&sample, &command);
#endif

#if defined(EW_FIRMWARE_USE_REAL_STM32_GPIO_INIT)
    real_gpio_ready = firmware_stm32f401re_real_gpio_init();
#endif

    if (board_profile_is_valid(board_profile_default()) &&
        rtos_task_model_is_valid() &&
        result.status_requested &&
        response_format_status(response, sizeof(response), firmware_last_state, &sample) &&
        firmware_alarm_output_self_check() &&
        firmware_stm32_gpio_init_self_check() &&
        freertos_ready &&
        real_gpio_ready) {
        firmware_self_check = 1;
    } else {
        firmware_self_check = -1;
    }

#if defined(EW_FIRMWARE_USE_FREERTOS) && defined(EW_FIRMWARE_START_FREERTOS_SCHEDULER)
    if (freertos_ready && !freertos_rtos_port_start_scheduler(&firmware_rtos_context)) {
        /* 正常情况下调度器启动后不应返回；返回说明启动失败或端口尚未真正接好。 */
        firmware_self_check = -2;
    }
#endif

    while (1) {
    }
}
