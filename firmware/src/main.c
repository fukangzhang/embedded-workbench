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
#include "embedded_workbench/serial_command_pump.h"
#include "embedded_workbench/serial_command_service.h"
#include "embedded_workbench/sensor_sample.h"
#include "embedded_workbench/sequence_sensor_source.h"
#include "embedded_workbench/stm32_board_gpio_init.h"
#include "embedded_workbench/stm32_board_usart2_init.h"
#include "embedded_workbench/stm32_usart_serial_io.h"
#include "embedded_workbench/stm32f401re_gpio_bindings.h"

/* firmware/main.c 当前不是最终产品逻辑，而是固件骨架自检入口。
 * 它把 app/drivers/bsp/FreeRTOS 链路尽量串起来，证明目标固件能编译、链接并进入 main。 */

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
static stm32_usart_serial_io_context_t firmware_usart2_serial_io = {0};
static serial_command_service_t firmware_usart2_command_service = {0};
static bool firmware_usart2_command_loop_ready = false;

static stm32_usart_serial_io_context_t firmware_freertos_usart2_serial_io = {0};
static alarm_config_t firmware_usart2_command_config;
static alarm_state_t firmware_usart2_command_state = ALARM_STATE_NORMAL;
static sensor_sample_t firmware_usart2_command_sample;
static char firmware_usart2_rx_buffer[96];
static char firmware_usart2_line_buffer[96];
static char firmware_usart2_response_buffer[256];

/* 下面这组不是 STM32 真实地址，而是固件自检用的模拟寄存器。
 * 真实地址绑定在 stm32f401re_gpio_bindings 模块里，当前 main 仍然保持可构建、可链接的安全自检。 */
static volatile uint32_t firmware_rcc_ahb1enr = 0u;
static volatile uint32_t firmware_rcc_apb1enr = 0u;
static stm32_gpio_registers_t firmware_gpioa_registers = {0};
static stm32_gpio_registers_t firmware_gpiob_registers = {0};
static stm32_usart_registers_t firmware_usart2_registers = {0};

typedef struct {
    const char *text;
    size_t index;
} firmware_text_reader_t;

static bool firmware_text_reader_read(void *context, char *byte_out)
{
    firmware_text_reader_t *reader = (firmware_text_reader_t *)context;

    if (reader == 0 || byte_out == 0 || reader->text == 0 || reader->text[reader->index] == '\0') {
        return false;
    }

    *byte_out = reader->text[reader->index];
    reader->index++;

    return true;
}

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

static bool gpio_pin_is_alternate(const stm32_gpio_registers_t *registers, unsigned int pin)
{
    uint32_t mode = 0u;

    if (registers == 0 || pin > 15u) {
        return false;
    }

    /* MODER 每个 pin 占两位，10 表示 alternate function 模式。 */
    mode = (registers->moder >> (pin * 2u)) & 3u;

    return mode == 2u;
}

static bool gpio_pin_has_alternate_function(
    const stm32_gpio_registers_t *registers,
    unsigned int pin,
    uint32_t expected_alternate_function)
{
    uint32_t alternate_register = 0u;
    uint32_t shift = 0u;

    if (registers == 0 || pin > 15u || expected_alternate_function > 15u) {
        return false;
    }

    /* AFRL 管 pin0-7，AFRH 管 pin8-15；每个 pin 占四位，AF7 就写入数值 7。 */
    alternate_register = pin < 8u ? registers->afrl : registers->afrh;
    shift = (pin % 8u) * 4u;

    return ((alternate_register >> shift) & 15u) == expected_alternate_function;
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

static bool firmware_stm32_usart2_init_self_check(void)
{
    size_t usart_clock_count = 0u;
    const stm32_rcc_usart_clock_peripheral_t *usart_clock_peripherals =
        stm32f401re_usart_clock_peripherals(&usart_clock_count);
    board_pin_t tx_pin = stm32f401re_usart2_tx_pin();
    board_pin_t rx_pin = stm32f401re_usart2_rx_pin();
    stm32_rcc_gpio_clock_port_t gpio_clock_ports[] = {
        {"PA", 0u},
    };
    stm32_gpio_config_port_t gpio_ports[] = {
        {"PA", &firmware_gpioa_registers},
    };
    stm32_rcc_gpio_clock_context_t gpio_clock_context;
    stm32_gpio_config_context_t gpio_context;
    stm32_rcc_usart_clock_context_t usart_clock_context;
    stm32_usart_config_t usart_config = stm32_usart_config_default(SystemCoreClock, 9600u);

    /* USART2 自检仍然写模拟寄存器：
     * - GPIOA 模拟 PA2/PA3 的 MODER/AFRL
     * - APB1ENR 模拟 USART2 时钟 bit17
     * - usart2_registers 模拟 BRR/CR1
     * 这证明固件入口能串起板级 USART2 初始化链路，但不证明真实串口已经输出波形。 */
    firmware_rcc_ahb1enr = 0u;
    firmware_rcc_apb1enr = 0u;
    firmware_gpioa_registers.moder = 0u;
    firmware_gpioa_registers.otyper = 0u;
    firmware_gpioa_registers.ospeedr = 0u;
    firmware_gpioa_registers.pupdr = 0u;
    firmware_gpioa_registers.afrl = 0u;
    firmware_gpioa_registers.afrh = 0u;
    firmware_usart2_registers.sr = 0u;
    firmware_usart2_registers.dr = 0u;
    firmware_usart2_registers.brr = 0u;
    firmware_usart2_registers.cr1 = 0u;
    firmware_usart2_registers.cr2 = 0u;
    firmware_usart2_registers.cr3 = 0u;

    return stm32_rcc_gpio_clock_init(&gpio_clock_context, &firmware_rcc_ahb1enr, gpio_clock_ports, 1u) &&
           stm32_gpio_config_init(&gpio_context, gpio_ports, 1u) &&
           stm32_rcc_usart_clock_init(
               &usart_clock_context,
               &firmware_rcc_apb1enr,
               usart_clock_peripherals,
               usart_clock_count) &&
           stm32_board_usart2_init(
               &gpio_clock_context,
               &gpio_context,
               &usart_clock_context,
               &firmware_usart2_registers,
               &usart_config) &&
           (firmware_rcc_ahb1enr & (1u << 0u)) == (1u << 0u) &&
           (firmware_rcc_apb1enr & (1u << STM32F401RE_USART2_APB1ENR_BIT)) ==
               (1u << STM32F401RE_USART2_APB1ENR_BIT) &&
           gpio_pin_is_alternate(&firmware_gpioa_registers, tx_pin.pin) &&
           gpio_pin_is_alternate(&firmware_gpioa_registers, rx_pin.pin) &&
           gpio_pin_has_alternate_function(
               &firmware_gpioa_registers,
               tx_pin.pin,
               STM32F401RE_USART2_GPIO_AF) &&
           gpio_pin_has_alternate_function(
               &firmware_gpioa_registers,
               rx_pin.pin,
               STM32F401RE_USART2_GPIO_AF) &&
           firmware_usart2_registers.brr == 1667u &&
           (firmware_usart2_registers.cr1 & ((1u << 13u) | (1u << 3u) | (1u << 2u))) ==
               ((1u << 13u) | (1u << 3u) | (1u << 2u));
}

static bool firmware_usart2_command_service_self_check(void)
{
    serial_command_service_t service;
    alarm_config_t config = alarm_config_default();
    alarm_state_t state = ALARM_STATE_WARNING;
    sensor_sample_t sample = sensor_sample_make(360, 600u, 250u, 20u);
    char rx_buffer[64];
    char line_buffer[64];
    char response_buffer[256];
    const char command_text[] = "STATUS?\n";
    firmware_text_reader_t reader = {command_text, 0u};
    size_t bytes_read = 0u;
    serial_command_service_status_t status = SERIAL_COMMAND_SERVICE_STATUS_IDLE;

    /* 这一段把 USART2 I/O 适配器接到串口命令服务：
     * reader -> serial_command_pump -> serial_command_service -> stm32_usart_serial_io_write。
     * 仍然写模拟 USART2 寄存器，所以它验证调用链，不验证真实 USB-UART 输出。 */
    firmware_usart2_registers.sr = 1u << 7u;
    firmware_usart2_registers.dr = 0u;

    if (!stm32_usart_serial_io_init(
            &firmware_usart2_serial_io,
            &firmware_usart2_registers,
            STM32_USART_SERIAL_IO_DEFAULT_MAX_POLL_ATTEMPTS) ||
        !serial_command_service_init(
            &service,
            rx_buffer,
            sizeof(rx_buffer),
            line_buffer,
            sizeof(line_buffer),
            response_buffer,
            sizeof(response_buffer),
            &config,
            &state,
            &sample,
            stm32_usart_serial_io_write,
            &firmware_usart2_serial_io)) {
        return false;
    }

    status = serial_command_pump_poll(
        &service,
        firmware_text_reader_read,
        &reader,
        sizeof(command_text) - 1u,
        &bytes_read);

    return status == SERIAL_COMMAND_SERVICE_STATUS_RESPONSE_SENT &&
           bytes_read == sizeof(command_text) - 1u &&
           response_buffer[0] == 'O' &&
           response_buffer[1] == 'K' &&
           firmware_usart2_registers.dr == (uint32_t)'\n';
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

#if defined(EW_FIRMWARE_USE_REAL_STM32_USART2_COMMAND_LOOP)
static bool firmware_stm32f401re_real_usart2_command_loop_init(void)
{
    stm32_rcc_gpio_clock_context_t gpio_clock_context;
    stm32_gpio_config_context_t gpio_context;
    stm32_rcc_usart_clock_context_t usart_clock_context;
    size_t gpio_clock_count = 0u;
    size_t gpio_port_count = 0u;
    size_t usart_clock_count = 0u;
    const stm32_rcc_gpio_clock_port_t *gpio_clock_ports =
        stm32f401re_gpio_clock_ports(&gpio_clock_count);
    const stm32_gpio_config_port_t *gpio_ports =
        stm32f401re_gpio_config_ports(&gpio_port_count);
    const stm32_rcc_usart_clock_peripheral_t *usart_clock_peripherals =
        stm32f401re_usart_clock_peripherals(&usart_clock_count);
    stm32_usart_registers_t *usart2_registers = stm32f401re_usart2_registers();
    stm32_usart_config_t usart_config = stm32_usart_config_default(SystemCoreClock, 9600u);
    bool ready = false;

    firmware_usart2_command_loop_ready = false;
    firmware_usart2_command_config = alarm_config_default();
    firmware_usart2_command_state = ALARM_STATE_NORMAL;
    firmware_usart2_command_sample = sensor_sample_make(250, 500u, 300u, 20u);

    /* 这个路径会解引用 STM32F401RE 的真实 RCC/GPIO/USART2 地址。
     * 默认关闭，只在真实板卡 bring-up 或专门构建验证时启用。 */
    ready = stm32_rcc_gpio_clock_init(
                &gpio_clock_context,
                stm32f401re_rcc_ahb1enr(),
                gpio_clock_ports,
                gpio_clock_count) &&
            stm32_gpio_config_init(&gpio_context, gpio_ports, gpio_port_count) &&
            stm32_rcc_usart_clock_init(
                &usart_clock_context,
                stm32f401re_rcc_apb1enr(),
                usart_clock_peripherals,
                usart_clock_count) &&
            stm32_board_usart2_init(
                &gpio_clock_context,
                &gpio_context,
                &usart_clock_context,
                usart2_registers,
                &usart_config) &&
            stm32_usart_serial_io_init(
                &firmware_usart2_serial_io,
                usart2_registers,
                STM32_USART_SERIAL_IO_DEFAULT_MAX_POLL_ATTEMPTS) &&
            serial_command_service_init(
                &firmware_usart2_command_service,
                firmware_usart2_rx_buffer,
                sizeof(firmware_usart2_rx_buffer),
                firmware_usart2_line_buffer,
                sizeof(firmware_usart2_line_buffer),
                firmware_usart2_response_buffer,
                sizeof(firmware_usart2_response_buffer),
                &firmware_usart2_command_config,
                &firmware_usart2_command_state,
                &firmware_usart2_command_sample,
                stm32_usart_serial_io_write,
                &firmware_usart2_serial_io);

    firmware_usart2_command_loop_ready = ready;

    return ready;
}

static void firmware_stm32f401re_real_usart2_command_loop_poll(void)
{
    if (firmware_usart2_command_loop_ready) {
        (void)serial_command_pump_poll(
            &firmware_usart2_command_service,
            stm32_usart_serial_io_read_byte,
            &firmware_usart2_serial_io,
            16u,
            0);
    }
}
#endif

#if defined(EW_FIRMWARE_USE_FREERTOS)
static freertos_rtos_port_context_t firmware_rtos_context = {0};
static rtos_port_t firmware_rtos_port = {0};

/* Scheduler 构建目前还没有真实传感器驱动，所以先给采集任务一组固定样本。
 * 这组样本会让状态从 normal 走到 warning/alarm，方便后续板上观察告警输出链路。 */
static const sensor_sample_t firmware_freertos_demo_samples[] = {
    {250, 500u, 300u, 20u},
    {360, 600u, 250u, 20u},
    {460, 700u, 80u, 40u},
};
static sequence_sensor_source_t firmware_freertos_sequence_source = {0};
static sensor_source_t firmware_freertos_sensor_source = {0};

#if defined(EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER)
static bool firmware_stm32f401re_freertos_usart2_command_reader_init(void)
{
    stm32_rcc_gpio_clock_context_t gpio_clock_context;
    stm32_gpio_config_context_t gpio_context;
    stm32_rcc_usart_clock_context_t usart_clock_context;
    size_t gpio_clock_count = 0u;
    size_t gpio_port_count = 0u;
    size_t usart_clock_count = 0u;
    const stm32_rcc_gpio_clock_port_t *gpio_clock_ports =
        stm32f401re_gpio_clock_ports(&gpio_clock_count);
    const stm32_gpio_config_port_t *gpio_ports =
        stm32f401re_gpio_config_ports(&gpio_port_count);
    const stm32_rcc_usart_clock_peripheral_t *usart_clock_peripherals =
        stm32f401re_usart_clock_peripherals(&usart_clock_count);
    stm32_usart_registers_t *usart2_registers = stm32f401re_usart2_registers();
    stm32_usart_config_t usart_config = stm32_usart_config_default(SystemCoreClock, 9600u);

    /* FreeRTOS reader 使用独立 serial I/O context。
     * 现有 USART2 自检会临时把 firmware_usart2_serial_io 指向模拟寄存器，不能复用。 */
    if (!stm32_rcc_gpio_clock_init(
            &gpio_clock_context,
            stm32f401re_rcc_ahb1enr(),
            gpio_clock_ports,
            gpio_clock_count) ||
        !stm32_gpio_config_init(&gpio_context, gpio_ports, gpio_port_count) ||
        !stm32_rcc_usart_clock_init(
            &usart_clock_context,
            stm32f401re_rcc_apb1enr(),
            usart_clock_peripherals,
            usart_clock_count) ||
        !stm32_board_usart2_init(
            &gpio_clock_context,
            &gpio_context,
            &usart_clock_context,
            usart2_registers,
            &usart_config) ||
        !stm32_usart_serial_io_init(
            &firmware_freertos_usart2_serial_io,
            usart2_registers,
            STM32_USART_SERIAL_IO_DEFAULT_MAX_POLL_ATTEMPTS)) {
        return false;
    }

    firmware_rtos_context.command_read = stm32_usart_serial_io_read_byte;
    firmware_rtos_context.command_read_context = &firmware_freertos_usart2_serial_io;
    return true;
}
#endif

static bool firmware_freertos_runtime_context_init(void)
{
    size_t sample_count = sizeof(firmware_freertos_demo_samples) / sizeof(firmware_freertos_demo_samples[0]);

    if (!sequence_sensor_source_init(
            &firmware_freertos_sequence_source,
            firmware_freertos_demo_samples,
            sample_count,
            true)) {
        return false;
    }

    firmware_freertos_sensor_source = sequence_sensor_source_as_source(&firmware_freertos_sequence_source);

    /* FreeRTOS task 只拿 context 指针；scheduler 真正启动后，采集任务从这里读 source，
     * 输出任务从这里拿 sink，所以这些字段必须在 vTaskStartScheduler 前准备好。 */
    firmware_rtos_context.sensor_source = &firmware_freertos_sensor_source;
    firmware_rtos_context.alarm_output_sink = &firmware_alarm_output_sink;

#if defined(EW_FIRMWARE_USE_REAL_STM32_USART2_FREERTOS_COMMAND_READER)
    if (!firmware_stm32f401re_freertos_usart2_command_reader_init()) {
        return false;
    }
#endif

    return sensor_source_is_valid(&firmware_freertos_sensor_source);
}

static bool firmware_freertos_queue_self_check(const sensor_sample_t *sample, const command_t *command)
{
    rtos_response_message_t queued_response = {{0}};
    rtos_response_message_t received_response = {{0}};

    queued_response.text[0] = 'O';
    queued_response.text[1] = 'K';
    queued_response.text[2] = '\0';

    /* FreeRTOS 自检只验证队列和端口包装能互通，不在这里启动真正的调度器。 */
    return firmware_freertos_runtime_context_init() &&
           freertos_rtos_port_init(&firmware_rtos_port, &firmware_rtos_context) &&
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
    bool base_self_check_ready = false;
    bool real_usart2_command_loop_ready = true;

    command_init(&command);
    command.type = COMMAND_TYPE_GET_STATUS;

    /* main 当前是固件骨架自检入口：把核心纯逻辑串起来，给链接和启动流程一个可观察结果。 */
    result = command_handler_handle(&command, &config, &sample);
    firmware_last_state = alarm_state_update(ALARM_STATE_NORMAL, &config, &sample);

#if defined(EW_FIRMWARE_USE_FREERTOS)
    freertos_ready = firmware_freertos_queue_self_check(&sample, &command);
#endif

#if defined(EW_FIRMWARE_USE_REAL_STM32_GPIO_INIT)
    real_gpio_ready = firmware_stm32f401re_real_gpio_init();
#endif

    base_self_check_ready = board_profile_is_valid(board_profile_default()) &&
                            rtos_task_model_is_valid() &&
                            result.status_requested &&
                            response_format_status(response, sizeof(response), firmware_last_state, &sample) &&
                            firmware_alarm_output_self_check() &&
                            firmware_stm32_gpio_init_self_check() &&
                            firmware_stm32_usart2_init_self_check() &&
                            firmware_usart2_command_service_self_check() &&
                            freertos_ready &&
                            real_gpio_ready;

#if defined(EW_FIRMWARE_USE_REAL_STM32_USART2_COMMAND_LOOP)
    real_usart2_command_loop_ready =
        base_self_check_ready && firmware_stm32f401re_real_usart2_command_loop_init();
#endif

    if (base_self_check_ready && real_usart2_command_loop_ready) {
        firmware_self_check = 1;
    } else {
        firmware_self_check = -1;
    }

#if defined(EW_FIRMWARE_USE_FREERTOS) && defined(EW_FIRMWARE_START_FREERTOS_SCHEDULER)
    if (firmware_self_check == 1 && freertos_ready && !freertos_rtos_port_start_scheduler(&firmware_rtos_context)) {
        /* 正常情况下调度器启动后不应返回；返回说明启动失败或端口尚未真正接好。 */
        firmware_self_check = -2;
    }
#endif

    while (1) {
#if defined(EW_FIRMWARE_USE_REAL_STM32_USART2_COMMAND_LOOP)
        firmware_stm32f401re_real_usart2_command_loop_poll();
#endif
    }
}
