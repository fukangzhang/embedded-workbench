#include "embedded_workbench/command_handler.h"

/* command_handler 位于 parser 和 session 中间：
 * parser 只负责识别命令文本，handler 负责判断命令能否作用到当前 alarm_config。
 * 它不直接输出字符串，这样同一套业务结果可以被串口、host_sim 或测试复用。 */

static bool is_int16_value(int32_t value)
{
    return value >= -32768 && value <= 32767;
}

static bool is_uint16_value(int32_t value)
{
    return value >= 0 && value <= 65535;
}

static bool is_uint32_value(int32_t value)
{
    return value >= 0;
}

static command_handler_result_t make_result(command_result_t result)
{
    command_handler_result_t handler_result;

    /* 所有标志默认关闭，只在对应命令成功时打开。
     * 这样错误结果不会意外触发 status/config 等后续响应。 */
    handler_result.result = result;
    handler_result.config_changed = false;
    handler_result.status_requested = false;
    handler_result.config_requested = false;
    handler_result.alarm_clear_requested = false;

    return handler_result;
}

static bool apply_threshold(command_threshold_t threshold, int32_t value, alarm_config_t *config)
{
    alarm_config_t next = *config;

    /* 先改副本 next，而不是直接改 config。
     * 这样即使新值导致整体配置无效，也不会把半更新状态留给系统。 */
    switch (threshold) {
    case COMMAND_THRESHOLD_TEMP_WARNING_HIGH:
        if (!is_int16_value(value)) {
            return false;
        }
        next.temperature_warning_high_c_x10 = (int16_t)value;
        break;
    case COMMAND_THRESHOLD_TEMP_ALARM_HIGH:
        if (!is_int16_value(value)) {
            return false;
        }
        next.temperature_alarm_high_c_x10 = (int16_t)value;
        break;
    case COMMAND_THRESHOLD_HUMIDITY_WARNING_HIGH:
        if (!is_uint16_value(value)) {
            return false;
        }
        next.humidity_warning_high_rh_x10 = (uint16_t)value;
        break;
    case COMMAND_THRESHOLD_HUMIDITY_ALARM_HIGH:
        if (!is_uint16_value(value)) {
            return false;
        }
        next.humidity_alarm_high_rh_x10 = (uint16_t)value;
        break;
    case COMMAND_THRESHOLD_LIGHT_WARNING_LOW:
        if (!is_uint32_value(value)) {
            return false;
        }
        next.light_warning_low_lux = (uint32_t)value;
        break;
    case COMMAND_THRESHOLD_LIGHT_ALARM_LOW:
        if (!is_uint32_value(value)) {
            return false;
        }
        next.light_alarm_low_lux = (uint32_t)value;
        break;
    case COMMAND_THRESHOLD_SMOKE_WARNING:
        if (!is_uint16_value(value)) {
            return false;
        }
        next.smoke_warning_ppm = (uint16_t)value;
        break;
    case COMMAND_THRESHOLD_SMOKE_ALARM:
        if (!is_uint16_value(value)) {
            return false;
        }
        next.smoke_alarm_ppm = (uint16_t)value;
        break;
    case COMMAND_THRESHOLD_NONE:
    default:
        return false;
    }

    if (!alarm_config_is_valid(&next)) {
        return false;
    }

    /* 只有类型范围和阈值相对关系都通过后，才提交新配置。 */
    *config = next;
    return true;
}

command_handler_result_t command_handler_handle(
    const command_t *command,
    alarm_config_t *config)
{
    command_handler_result_t result;

    if (command == 0 || config == 0 || !alarm_config_is_valid(config)) {
        return make_result(COMMAND_RESULT_INVALID_COMMAND);
    }

    switch (command->type) {
    case COMMAND_TYPE_GET_STATUS:
        /* 查询命令不在这里格式化文本，只设置标志交给上层决定如何响应。 */
        result = make_result(COMMAND_RESULT_OK);
        result.status_requested = true;
        return result;
    case COMMAND_TYPE_GET_CONFIG:
        result = make_result(COMMAND_RESULT_OK);
        result.config_requested = true;
        return result;
    case COMMAND_TYPE_CLEAR_ALARM:
        result = make_result(COMMAND_RESULT_OK);
        result.alarm_clear_requested = true;
        return result;
    case COMMAND_TYPE_SET_THRESHOLD:
        result = make_result(COMMAND_RESULT_OK);
        if (!apply_threshold(command->threshold, command->value, config)) {
            return make_result(COMMAND_RESULT_INVALID_VALUE);
        }

        /* config_changed 表示配置已经成功提交，测试可用它区分“请求合法”和“确实修改”。 */
        result.config_changed = true;
        return result;
    case COMMAND_TYPE_INVALID:
    default:
        return make_result(COMMAND_RESULT_INVALID_COMMAND);
    }
}

const char *command_result_name(command_result_t result)
{
    switch (result) {
    case COMMAND_RESULT_OK:
        return "ok";
    case COMMAND_RESULT_INVALID_COMMAND:
        return "invalid-command";
    case COMMAND_RESULT_INVALID_VALUE:
        return "invalid-value";
    default:
        return "unknown";
    }
}
