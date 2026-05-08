#include <stdbool.h>

#include "embedded_workbench/rtos_port.h"

typedef struct {
    bool started;
    unsigned int sensor_sample_count;
    unsigned int command_count;
    unsigned int response_count;
} fake_rtos_context_t;

static bool fake_start(void *context)
{
    fake_rtos_context_t *fake = (fake_rtos_context_t *)context;
    fake->started = true;
    return true;
}

static bool fake_send_sensor_sample(void *context, const sensor_sample_t *sample)
{
    fake_rtos_context_t *fake = (fake_rtos_context_t *)context;
    (void)sample;
    fake->sensor_sample_count++;
    return true;
}

static bool fake_send_command(void *context, const command_t *command)
{
    fake_rtos_context_t *fake = (fake_rtos_context_t *)context;
    (void)command;
    fake->command_count++;
    return true;
}

static bool fake_receive_response(void *context, rtos_response_message_t *response)
{
    fake_rtos_context_t *fake = (fake_rtos_context_t *)context;
    response->text[0] = 'O';
    response->text[1] = 'K';
    response->text[2] = '\0';
    fake->response_count++;
    return true;
}

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_uint(unsigned int actual, unsigned int expected)
{
    return actual == expected ? 0 : 1;
}

int main(void)
{
    static const rtos_port_ops_t fake_ops = {
        fake_start,
        fake_send_sensor_sample,
        fake_send_command,
        fake_receive_response,
    };
    fake_rtos_context_t context = {false, 0u, 0u, 0u};
    rtos_port_t port = {&fake_ops, &context};
    sensor_sample_t sample = sensor_sample_make(250, 500u, 300u, 20u);
    sensor_sample_t invalid_sample = sensor_sample_make(SENSOR_TEMPERATURE_MAX_C_X10 + 1, 500u, 300u, 20u);
    command_t command;
    rtos_response_message_t response;

    command_init(&command);
    command.type = COMMAND_TYPE_GET_STATUS;

    /* fake 端口用于确认 rtos_port 包装层会调用 ops，并维护基本的输入校验。 */
    if (expect_true(rtos_port_is_valid(&port)) != 0) {
        return 1;
    }

    if (expect_true(rtos_port_start(&port)) != 0 ||
        expect_true(context.started) != 0) {
        return 2;
    }

    if (expect_true(rtos_port_send_sensor_sample(&port, &sample)) != 0 ||
        expect_uint(context.sensor_sample_count, 1u) != 0) {
        return 3;
    }

    if (expect_false(rtos_port_send_sensor_sample(&port, &invalid_sample)) != 0 ||
        expect_uint(context.sensor_sample_count, 1u) != 0) {
        return 4;
    }

    /* 无效采样/无效命令不能进入 fake 计数，说明包装层在委托前拦住了坏输入。 */
    if (expect_true(rtos_port_send_command(&port, &command)) != 0 ||
        expect_uint(context.command_count, 1u) != 0) {
        return 5;
    }

    command.type = COMMAND_TYPE_INVALID;
    if (expect_false(rtos_port_send_command(&port, &command)) != 0 ||
        expect_uint(context.command_count, 1u) != 0) {
        return 6;
    }

    if (expect_true(rtos_port_receive_response(&port, &response)) != 0 ||
        expect_uint(context.response_count, 1u) != 0 ||
        response.text[0] != 'O' ||
        response.text[1] != 'K') {
        return 7;
    }

    if (expect_false(rtos_port_start(0)) != 0) {
        return 8;
    }

    return 0;
}
