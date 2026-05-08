#include "embedded_workbench/alarm_output_digital_sink.h"

static digital_output_level_t level_from_enabled(bool enabled)
{
    return digital_output_level_from_bool(enabled);
}

static bool set_indicator(
    void *context,
    alarm_output_indicator_t indicator,
    uint16_t period_ms)
{
    alarm_output_digital_sink_context_t *digital_context =
        (alarm_output_digital_sink_context_t *)context;
    bool enabled = indicator != ALARM_OUTPUT_INDICATOR_OFF;

    (void)period_ms;

    return digital_output_write(
        digital_context->digital_output,
        &digital_context->profile->alarm_led,
        level_from_enabled(enabled));
}

static bool set_buzzer(void *context, bool enabled)
{
    alarm_output_digital_sink_context_t *digital_context =
        (alarm_output_digital_sink_context_t *)context;

    return digital_output_write(
        digital_context->digital_output,
        &digital_context->profile->alarm_buzzer,
        level_from_enabled(enabled));
}

static bool set_actuator(void *context, bool enabled)
{
    alarm_output_digital_sink_context_t *digital_context =
        (alarm_output_digital_sink_context_t *)context;

    return digital_output_write(
        digital_context->digital_output,
        &digital_context->profile->alarm_actuator,
        level_from_enabled(enabled));
}

bool alarm_output_digital_sink_init(
    alarm_output_sink_t *sink,
    alarm_output_digital_sink_context_t *context,
    digital_output_controller_t *digital_output,
    const board_profile_t *profile)
{
    static const alarm_output_sink_ops_t ops = {
        set_indicator,
        set_buzzer,
        set_actuator,
    };

    if (sink == 0 ||
        context == 0 ||
        digital_output == 0 ||
        !board_profile_is_valid(profile)) {
        return false;
    }

    context->digital_output = digital_output;
    context->profile = profile;
    sink->ops = &ops;
    sink->context = context;

    return true;
}
