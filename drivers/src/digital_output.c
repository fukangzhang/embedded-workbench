#include "embedded_workbench/digital_output.h"

/* digital_output 是通用数字输出抽象。
 * 上层给它 board_pin_t 和 high/low，底层后端决定这是记录到测试 context，还是写 STM32 BSRR。 */

static bool pin_is_valid(const board_pin_t *pin)
{
    return pin != 0 &&
           pin->port != 0 &&
           pin->pin <= 15u &&
           pin->function != 0;
}

static bool controller_is_ready(const digital_output_controller_t *controller)
{
    return controller != 0 &&
           controller->ops != 0 &&
           controller->ops->write != 0;
}

static bool level_is_valid(digital_output_level_t level)
{
    return level == DIGITAL_OUTPUT_LEVEL_LOW ||
           level == DIGITAL_OUTPUT_LEVEL_HIGH;
}

bool digital_output_write(
    digital_output_controller_t *controller,
    const board_pin_t *pin,
    digital_output_level_t level)
{
    /* 这是驱动层的公共防线：检查控制器、引脚和电平后，再委托给具体板级实现。 */
    if (!controller_is_ready(controller) ||
        !pin_is_valid(pin) ||
        !level_is_valid(level)) {
        return false;
    }

    return controller->ops->write(controller->context, pin, level);
}

digital_output_level_t digital_output_level_from_bool(bool enabled)
{
    return enabled ? DIGITAL_OUTPUT_LEVEL_HIGH : DIGITAL_OUTPUT_LEVEL_LOW;
}

const char *digital_output_level_name(digital_output_level_t level)
{
    switch (level) {
    case DIGITAL_OUTPUT_LEVEL_LOW:
        return "low";
    case DIGITAL_OUTPUT_LEVEL_HIGH:
        return "high";
    default:
        return "unknown";
    }
}
