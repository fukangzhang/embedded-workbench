#include "embedded_workbench/rtos_port.h"

bool rtos_port_is_valid(const rtos_port_t *port)
{
    return port != 0 &&
           port->ops != 0 &&
           port->ops->start != 0 &&
           port->ops->send_sensor_sample != 0 &&
           port->ops->send_command != 0 &&
           port->ops->receive_response != 0;
}

bool rtos_port_start(const rtos_port_t *port)
{
    if (!rtos_port_is_valid(port)) {
        return false;
    }

    return port->ops->start(port->context);
}

bool rtos_port_send_sensor_sample(const rtos_port_t *port, const sensor_sample_t *sample)
{
    if (!rtos_port_is_valid(port) || !sensor_sample_is_valid(sample)) {
        return false;
    }

    return port->ops->send_sensor_sample(port->context, sample);
}

bool rtos_port_send_command(const rtos_port_t *port, const command_t *command)
{
    if (!rtos_port_is_valid(port) || command == 0 || command->type == COMMAND_TYPE_INVALID) {
        return false;
    }

    return port->ops->send_command(port->context, command);
}

bool rtos_port_receive_response(const rtos_port_t *port, rtos_response_message_t *response)
{
    if (!rtos_port_is_valid(port) || response == 0) {
        return false;
    }

    return port->ops->receive_response(port->context, response);
}
