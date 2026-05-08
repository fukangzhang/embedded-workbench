#include "embedded_workbench/board_profile.h"

static const board_profile_t profiles[] = {
    /* 两块 NUCLEO 板先共用一组项目占位引脚，后续接真实外设时只需要更新 profile。 */
    {
        BOARD_ID_NUCLEO_F401RE,
        "NUCLEO-F401RE",
        "STM32F401RE",
        "on-board ST-LINK",
        "Arduino Uno V3, ST morpho",
        "USART2",
        "I2C1",
        {"PA", 5u, "LD2 user LED / alarm indicator"},
        {"PB", 6u, "external buzzer placeholder"},
        {"PB", 7u, "external actuator enable placeholder"},
    },
    {
        BOARD_ID_NUCLEO_F103RB,
        "NUCLEO-F103RB",
        "STM32F103RB",
        "on-board ST-LINK",
        "Arduino Uno V3, ST morpho",
        "USART2",
        "I2C1",
        {"PA", 5u, "LD2 user LED / alarm indicator"},
        {"PB", 6u, "external buzzer placeholder"},
        {"PB", 7u, "external actuator enable placeholder"},
    },
};

const board_profile_t *board_profile_default(void)
{
    return &profiles[0];
}

const board_profile_t *board_profile_find(board_id_t id)
{
    size_t index = 0;

    /* profile 数量很小，线性查找最直观，也避免引入额外表结构。 */
    for (index = 0; index < board_profile_count(); index++) {
        if (profiles[index].id == id) {
            return &profiles[index];
        }
    }

    return 0;
}

size_t board_profile_count(void)
{
    return sizeof(profiles) / sizeof(profiles[0]);
}

const board_profile_t *board_profile_at(size_t index)
{
    if (index >= board_profile_count()) {
        return 0;
    }

    return &profiles[index];
}

static bool pin_is_valid(const board_pin_t *pin)
{
    return pin != 0 &&
           pin->port != 0 &&
           pin->pin <= 15u &&
           pin->function != 0;
}

bool board_profile_is_valid(const board_profile_t *profile)
{
    /* profile 是后续 BSP/driver 初始化的事实来源，必须确保所有字符串和关键引脚都存在。 */
    return profile != 0 &&
           profile->name != 0 &&
           profile->mcu != 0 &&
           profile->debug_probe != 0 &&
           profile->connectors != 0 &&
           profile->console_uart != 0 &&
           profile->sensor_bus != 0 &&
           pin_is_valid(&profile->alarm_led) &&
           pin_is_valid(&profile->alarm_buzzer) &&
           pin_is_valid(&profile->alarm_actuator);
}
