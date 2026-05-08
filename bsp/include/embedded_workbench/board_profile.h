#ifndef EMBEDDED_WORKBENCH_BOARD_PROFILE_H
#define EMBEDDED_WORKBENCH_BOARD_PROFILE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    BOARD_ID_NUCLEO_F401RE = 0,
    BOARD_ID_NUCLEO_F103RB = 1
} board_id_t;

typedef struct {
    const char *port;
    unsigned int pin;
    const char *function;
} board_pin_t;

typedef struct {
    board_id_t id;
    const char *name;
    const char *mcu;
    const char *debug_probe;
    const char *connectors;
    const char *console_uart;
    const char *sensor_bus;
    board_pin_t alarm_led;
    board_pin_t alarm_buzzer;
    board_pin_t alarm_actuator;
} board_profile_t;

const board_profile_t *board_profile_default(void);
const board_profile_t *board_profile_find(board_id_t id);
size_t board_profile_count(void);
const board_profile_t *board_profile_at(size_t index);
bool board_profile_is_valid(const board_profile_t *profile);

#endif
