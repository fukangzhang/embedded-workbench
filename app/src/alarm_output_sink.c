#include "embedded_workbench/alarm_output_sink.h"

static bool sink_is_ready(const alarm_output_sink_t *sink)
{
    return sink != 0 &&
           sink->ops != 0 &&
           sink->ops->set_indicator != 0 &&
           sink->ops->set_buzzer != 0 &&
           sink->ops->set_actuator != 0;
}

bool alarm_output_sink_apply(
    alarm_output_sink_t *sink,
    const alarm_output_command_t *command)
{
    if (!sink_is_ready(sink) || command == 0) {
        return false;
    }

    if (!sink->ops->set_indicator(sink->context, command->indicator, command->period_ms)) {
        return false;
    }
    if (!sink->ops->set_buzzer(sink->context, command->buzzer_enabled)) {
        return false;
    }
    if (!sink->ops->set_actuator(sink->context, command->actuator_enabled)) {
        return false;
    }

    return true;
}
