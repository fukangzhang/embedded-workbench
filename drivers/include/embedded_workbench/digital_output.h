#ifndef EMBEDDED_WORKBENCH_DIGITAL_OUTPUT_H
#define EMBEDDED_WORKBENCH_DIGITAL_OUTPUT_H

#include <stdbool.h>

#include "embedded_workbench/board_profile.h"

typedef enum {
    /* 逻辑低电平。真实含义由具体硬件电路决定，通常是 0V 附近。 */
    DIGITAL_OUTPUT_LEVEL_LOW = 0,
    /* 逻辑高电平。真实电压取决于 MCU IO 电压。 */
    DIGITAL_OUTPUT_LEVEL_HIGH = 1
} digital_output_level_t;

typedef bool (*digital_output_write_fn)(
    void *context,
    const board_pin_t *pin,
    digital_output_level_t level);

typedef struct {
    /* 单一 write 操作足够表达“把某个板级引脚写成高/低”。 */
    digital_output_write_fn write;
} digital_output_ops_t;

typedef struct {
    /* ops + context 让同一套上层逻辑可接主机模拟后端或真实 STM32 GPIO 后端。 */
    const digital_output_ops_t *ops;
    void *context;
} digital_output_controller_t;

/* 写一个板级引脚的数字电平。
 * 这里的 pin 来自 board_profile；digital_output 本身不保存目标板引脚表。 */
bool digital_output_write(
    digital_output_controller_t *controller,
    const board_pin_t *pin,
    digital_output_level_t level);
digital_output_level_t digital_output_level_from_bool(bool enabled);
const char *digital_output_level_name(digital_output_level_t level);

#endif
