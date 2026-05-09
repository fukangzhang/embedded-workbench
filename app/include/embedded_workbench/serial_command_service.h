#ifndef EMBEDDED_WORKBENCH_SERIAL_COMMAND_SERVICE_H
#define EMBEDDED_WORKBENCH_SERIAL_COMMAND_SERVICE_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/command_session.h"
#include "embedded_workbench/serial_line.h"

typedef enum {
    SERIAL_COMMAND_SERVICE_STATUS_IDLE = 0,
    SERIAL_COMMAND_SERVICE_STATUS_RESPONSE_SENT = 1,
    SERIAL_COMMAND_SERVICE_STATUS_OVERFLOW = 2,
    SERIAL_COMMAND_SERVICE_STATUS_ERROR = 3
} serial_command_service_status_t;

typedef bool (*serial_command_service_write_fn)(void *context, const char *data, size_t length);

typedef struct {
    /* reader 使用 rx_buffer 保存尚未完成的一行；line_buffer 保存完成后的命令文本。 */
    serial_line_reader_t reader;
    char *line_buffer;
    size_t line_capacity;
    char *response_buffer;
    size_t response_capacity;
    command_session_t session;
    serial_command_service_write_fn write;
    void *write_context;
} serial_command_service_t;

/* 初始化串口命令服务。
 *
 * 所有 buffer 都由调用者提供，方便 MCU 固件使用静态内存，不依赖 malloc。
 * rx_buffer 和 line_buffer 需要分开：serial_line 完成一行后会 reset 内部 rx_buffer。 */
bool serial_command_service_init(
    serial_command_service_t *service,
    char *rx_buffer,
    size_t rx_capacity,
    char *line_buffer,
    size_t line_capacity,
    char *response_buffer,
    size_t response_capacity,
    alarm_config_t *config,
    alarm_state_t *state,
    const sensor_sample_t *sample,
    serial_command_service_write_fn write,
    void *write_context);

/* 喂入 1 个串口字节。
 *
 * 返回 IDLE 表示还没有完整命令；RESPONSE_SENT 表示已经解析一行并发送响应。
 * OVERFLOW 表示当前行太长，服务已复位行缓冲，调用者可以继续接收下一行。 */
serial_command_service_status_t serial_command_service_feed(serial_command_service_t *service, char byte);

#endif
