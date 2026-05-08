#ifndef EMBEDDED_WORKBENCH_DIGITAL_OUTPUT_H
#define EMBEDDED_WORKBENCH_DIGITAL_OUTPUT_H

#include <stdbool.h>

#include "embedded_workbench/board_profile.h"

typedef enum {
    DIGITAL_OUTPUT_LEVEL_LOW = 0,
    DIGITAL_OUTPUT_LEVEL_HIGH = 1
} digital_output_level_t;

typedef bool (*digital_output_write_fn)(
    void *context,
    const board_pin_t *pin,
    digital_output_level_t level);

typedef struct {
    digital_output_write_fn write;
} digital_output_ops_t;

typedef struct {
    const digital_output_ops_t *ops;
    void *context;
} digital_output_controller_t;

bool digital_output_write(
    digital_output_controller_t *controller,
    const board_pin_t *pin,
    digital_output_level_t level);
digital_output_level_t digital_output_level_from_bool(bool enabled);
const char *digital_output_level_name(digital_output_level_t level);

#endif
