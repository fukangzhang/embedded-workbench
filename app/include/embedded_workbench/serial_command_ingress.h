#ifndef EMBEDDED_WORKBENCH_SERIAL_COMMAND_INGRESS_H
#define EMBEDDED_WORKBENCH_SERIAL_COMMAND_INGRESS_H

#include <stdbool.h>
#include <stddef.h>

#include "embedded_workbench/command_parser.h"
#include "embedded_workbench/serial_line.h"

typedef enum {
    SERIAL_COMMAND_INGRESS_STATUS_IDLE = 0,
    SERIAL_COMMAND_INGRESS_STATUS_COMMAND_SUBMITTED = 1,
    SERIAL_COMMAND_INGRESS_STATUS_OVERFLOW = 2,
    SERIAL_COMMAND_INGRESS_STATUS_ERROR = 3
} serial_command_ingress_status_t;

typedef bool (*serial_command_ingress_submit_fn)(void *context, const command_t *command);

typedef struct {
    /* reader 负责逐字节组装一行，ingress 不自己重新实现 CR/LF 细节。 */
    serial_line_reader_t reader;
    /* line_buffer 保存完成后的命令行，随后交给 command_parse。 */
    char *line_buffer;
    size_t line_capacity;
    /* submit 是下一跳：主机测试可捕获命令，FreeRTOS 后续可发送到 command_queue。 */
    serial_command_ingress_submit_fn submit;
    void *submit_context;
} serial_command_ingress_t;

bool serial_command_ingress_init(
    serial_command_ingress_t *ingress,
    char *rx_buffer,
    size_t rx_capacity,
    char *line_buffer,
    size_t line_capacity,
    serial_command_ingress_submit_fn submit,
    void *submit_context);

/* 喂入一个串口字节。
 * 完整行到达后，无论解析是否成功都会提交一个 command_t：
 * - 解析成功：提交具体命令。
 * - 解析失败：提交 COMMAND_TYPE_INVALID，方便后续 responder 统一生成 ERR 响应。 */
serial_command_ingress_status_t serial_command_ingress_feed(serial_command_ingress_t *ingress, char byte);

#endif
