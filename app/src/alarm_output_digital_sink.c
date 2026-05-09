#include "embedded_workbench/alarm_output_digital_sink.h"

/* alarm_output_digital_sink 把告警输出接口适配到 digital_output。
 * 它知道 profile 里的三个输出脚，但仍然不直接操作 STM32 寄存器。 */

static digital_output_level_t level_from_enabled(bool enabled)
{
    /* 告警输出层用 bool 表达开/关，digital_output 层用 LOW/HIGH 表达电平。
     * 这个小转换让两个层次保持各自的语言。 */
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

    /* digital sink 只关心“灯此刻是否需要亮”。
     * 真正的闪烁节拍由 alarm_output_timing 或上层任务先计算好。 */
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

    /* context 保存真实依赖，sink 只暴露统一 ops。
     * 这就是 C 里常见的“函数指针 + context”接口模式。 */
    context->digital_output = digital_output;
    context->profile = profile;
    sink->ops = &ops;
    sink->context = context;

    return true;
}
