#ifndef EMBEDDED_WORKBENCH_RTOS_PORT_H
#define EMBEDDED_WORKBENCH_RTOS_PORT_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/sensor_sample.h"

typedef struct {
    char text[128];
} rtos_response_message_t;

typedef struct {
    bool (*start)(void *context);
    bool (*send_sensor_sample)(void *context, const sensor_sample_t *sample);
    bool (*send_command)(void *context, const command_t *command);
    bool (*receive_response)(void *context, rtos_response_message_t *response);
} rtos_port_ops_t;

typedef struct {
    const rtos_port_ops_t *ops;
    void *context;
} rtos_port_t;

bool rtos_port_is_valid(const rtos_port_t *port);
bool rtos_port_start(const rtos_port_t *port);
bool rtos_port_send_sensor_sample(const rtos_port_t *port, const sensor_sample_t *sample);
bool rtos_port_send_command(const rtos_port_t *port, const command_t *command);
bool rtos_port_receive_response(const rtos_port_t *port, rtos_response_message_t *response);

#endif
