#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/board_profile.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_size(size_t actual, size_t expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

int main(void)
{
    const board_profile_t *profile = 0;

    /* 当前 profile 表里有 F401RE 主目标板和一个预留 F103RB。 */
    if (expect_size(board_profile_count(), 2u) != 0) {
        return 1;
    }

    /* 默认板卡必须是当前项目主线使用的 NUCLEO-F401RE。 */
    profile = board_profile_default();
    if (profile == 0 ||
        expect_true(board_profile_is_valid(profile)) != 0 ||
        expect_string(profile->name, "NUCLEO-F401RE") != 0 ||
        expect_string(profile->mcu, "STM32F401RE") != 0 ||
        expect_string(profile->console_uart, "USART2") != 0 ||
        expect_string(profile->sensor_bus, "I2C1") != 0 ||
        expect_string(profile->alarm_actuator.port, "PB") != 0 ||
        profile->alarm_actuator.pin != 7u) {
        return 2;
    }

    /* find 用 id 查找目标板，后续切板或比较不同 NUCLEO 时会依赖它。 */
    profile = board_profile_find(BOARD_ID_NUCLEO_F103RB);
    if (profile == 0 ||
        expect_true(board_profile_is_valid(profile)) != 0 ||
        expect_string(profile->name, "NUCLEO-F103RB") != 0 ||
        expect_string(profile->mcu, "STM32F103RB") != 0) {
        return 3;
    }

    profile = board_profile_at(99u);
    if (profile != 0) {
        return 4;
    }

    profile = board_profile_find((board_id_t)99);
    if (profile != 0) {
        return 5;
    }

    /* 空 profile 不合法，避免 BSP/driver 初始化继续访问空指针。 */
    if (expect_true(!board_profile_is_valid(0)) != 0) {
        return 6;
    }

    return 0;
}
