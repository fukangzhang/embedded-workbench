#ifndef EMBEDDED_WORKBENCH_BOARD_PROFILE_H
#define EMBEDDED_WORKBENCH_BOARD_PROFILE_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    /* 当前主要目标板，NUCLEO-F401RE。 */
    BOARD_ID_NUCLEO_F401RE = 0,
    /* 预留的相近 NUCLEO 板卡，用于后续比较/迁移。 */
    BOARD_ID_NUCLEO_F103RB = 1
} board_id_t;

typedef struct {
    /* 端口名用 "PA"/"PB" 这种人可读名字，避免 app 层直接接触寄存器地址。 */
    const char *port;
    /* GPIO pin 编号，范围通常是 0..15。 */
    unsigned int pin;
    /* 该引脚在项目中的用途，例如 alarm_led。 */
    const char *function;
} board_pin_t;

/* 板卡描述只记录项目关心的硬件事实，不在这里做实际初始化。 */
typedef struct {
    /* id/name/mcu 让代码、文档、测试能明确当前目标板。 */
    board_id_t id;
    const char *name;
    const char *mcu;
    /* 下面这些字符串是 bring-up 需要知道的连接信息，不是驱动代码。 */
    const char *debug_probe;
    const char *connectors;
    const char *console_uart;
    const char *sensor_bus;
    /* 项目当前关心的三个告警输出脚。 */
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
