#ifndef EMBEDDED_WORKBENCH_SERIAL_LINE_H
#define EMBEDDED_WORKBENCH_SERIAL_LINE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    SERIAL_LINE_STATUS_IDLE = 0,
    SERIAL_LINE_STATUS_LINE_READY = 1,
    SERIAL_LINE_STATUS_OVERFLOW = 2,
    SERIAL_LINE_STATUS_ERROR = 3
} serial_line_status_t;

typedef struct {
    /* buffer 由调用者提供，模块只借用它，不负责分配或释放。
     * 这符合嵌入式常见做法：静态内存可控，避免在中断/任务里动态分配。 */
    char *buffer;
    size_t capacity;
    size_t length;
    /* 预留 CRLF 状态位，方便兼容常见终端的 "\r\n" 换行组合。 */
    bool previous_was_cr;
} serial_line_reader_t;

bool serial_line_reader_init(serial_line_reader_t *reader, char *buffer, size_t capacity);
void serial_line_reader_reset(serial_line_reader_t *reader);
size_t serial_line_reader_length(const serial_line_reader_t *reader);

/* 每次喂入 1 个字节：
 * - 普通字符进入内部缓冲区
 * - '\n' 或 '\r' 尝试完成一行
 * - 行完成后复制到 line_out，并以 '\0' 结尾，便于 command_parser 继续处理 */
serial_line_status_t serial_line_feed(
    serial_line_reader_t *reader,
    char byte,
    char *line_out,
    size_t line_out_size,
    size_t *line_length_out);

#endif
