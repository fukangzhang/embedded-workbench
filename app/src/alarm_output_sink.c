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

    /* 按“指示灯 -> 蜂鸣器 -> 执行器”的顺序输出。
     * 如果某一步失败，后续输出不再继续，调用者能知道本次命令没有完整落地。 */
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
