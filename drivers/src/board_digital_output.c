#include "embedded_workbench/board_digital_output.h"

static bool text_equals(const char *left, const char *right)
{
    if (left == 0 || right == 0) {
        return false;
    }

    while (*left != '\0' && *right != '\0') {
        if (*left != *right) {
            return false;
        }
        left++;
        right++;
    }

    return *left == '\0' && *right == '\0';
}

static bool pin_matches(const board_pin_t *left, const board_pin_t *right)
{
    return left != 0 &&
           right != 0 &&
           left->pin == right->pin &&
           text_equals(left->port, right->port);
}

static digital_output_level_t *level_for_pin(
    board_digital_output_context_t *context,
    const board_pin_t *pin)
{
    if (context == 0 || context->profile == 0 || pin == 0) {
        return 0;
    }

    if (pin_matches(pin, &context->profile->alarm_led)) {
        return &context->alarm_led_level;
    }
    if (pin_matches(pin, &context->profile->alarm_buzzer)) {
        return &context->alarm_buzzer_level;
    }
    if (pin_matches(pin, &context->profile->alarm_actuator)) {
        return &context->alarm_actuator_level;
    }

    return 0;
}

static bool board_write(
    void *context,
    const board_pin_t *pin,
    digital_output_level_t level)
{
    board_digital_output_context_t *board_context =
        (board_digital_output_context_t *)context;
    digital_output_level_t *stored_level = level_for_pin(board_context, pin);

    if (stored_level == 0) {
        return false;
    }

    *stored_level = level;
    board_context->write_count++;

    return true;
}

bool board_digital_output_init(
    digital_output_controller_t *controller,
    board_digital_output_context_t *context,
    const board_profile_t *profile)
{
    static const digital_output_ops_t ops = {
        board_write,
    };

    if (controller == 0 || context == 0 || !board_profile_is_valid(profile)) {
        return false;
    }

    context->profile = profile;
    context->alarm_led_level = DIGITAL_OUTPUT_LEVEL_LOW;
    context->alarm_buzzer_level = DIGITAL_OUTPUT_LEVEL_LOW;
    context->alarm_actuator_level = DIGITAL_OUTPUT_LEVEL_LOW;
    context->write_count = 0u;

    controller->ops = &ops;
    controller->context = context;

    return true;
}

bool board_digital_output_get_level(
    const board_digital_output_context_t *context,
    const board_pin_t *pin,
    digital_output_level_t *level)
{
    digital_output_level_t *stored_level =
        level_for_pin((board_digital_output_context_t *)context, pin);

    if (stored_level == 0 || level == 0) {
        return false;
    }

    *level = *stored_level;

    return true;
}
