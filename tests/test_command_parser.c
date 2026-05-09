#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/command_parser.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_int(int actual, int expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

int main(void)
{
    command_t command;

    /* 先确认初始化会把命令放到安全的 INVALID 状态。
     * 这样解析失败时，上层不会误用上一次残留的命令字段。 */
    command_init(&command);
    if (expect_int(command.type, COMMAND_TYPE_INVALID) != 0) {
        return 1;
    }

    /* 查询命令没有参数，但允许常见的 CRLF 行尾。 */
    if (expect_true(command_parse("STATUS?\r\n", &command)) != 0 ||
        expect_int(command.type, COMMAND_TYPE_GET_STATUS) != 0) {
        return 2;
    }

    /* 前后空格应被忽略，方便人手工在终端输入命令。 */
    if (expect_true(command_parse(" CONFIG? ", &command)) != 0 ||
        expect_int(command.type, COMMAND_TYPE_GET_CONFIG) != 0) {
        return 3;
    }

    if (expect_true(command_parse("CLEAR_ALARM", &command)) != 0 ||
        expect_int(command.type, COMMAND_TYPE_CLEAR_ALARM) != 0) {
        return 4;
    }

    if (expect_true(command_parse("SET TEMP_WARN 350", &command)) != 0 ||
        expect_int(command.type, COMMAND_TYPE_SET_THRESHOLD) != 0 ||
        expect_int(command.threshold, COMMAND_THRESHOLD_TEMP_WARNING_HIGH) != 0 ||
        expect_int(command.value, 350) != 0) {
        return 5;
    }

    /* SET 覆盖正数、负数和不同阈值名，确保 token 解析和 int32 解析都被验证。 */
    if (expect_true(command_parse("SET LIGHT_ALARM_LOW 30", &command)) != 0 ||
        expect_int(command.threshold, COMMAND_THRESHOLD_LIGHT_ALARM_LOW) != 0 ||
        expect_int(command.value, 30) != 0) {
        return 6;
    }

    if (expect_true(command_parse("SET TEMP_ALARM -50", &command)) != 0 ||
        expect_int(command.threshold, COMMAND_THRESHOLD_TEMP_ALARM_HIGH) != 0 ||
        expect_int(command.value, -50) != 0) {
        return 7;
    }

    if (expect_true(command_parse("SAMPLE 360 600 250 20", &command)) != 0 ||
        expect_int(command.type, COMMAND_TYPE_SET_SAMPLE) != 0 ||
        expect_int(command.sample_temperature_c_x10, 360) != 0 ||
        expect_int(command.sample_humidity_rh_x10, 600) != 0 ||
        expect_int(command.sample_light_lux, 250) != 0 ||
        expect_int(command.sample_smoke_ppm, 20) != 0) {
        return 8;
    }

    if (expect_false(command_parse("STATUS? NOW", &command)) != 0) {
        return 9;
    }

    /* 下面这些失败用例分别覆盖未知阈值、非数字、整数溢出和空指针。 */
    if (expect_false(command_parse("SET UNKNOWN 1", &command)) != 0) {
        return 10;
    }

    if (expect_false(command_parse("SET TEMP_WARN abc", &command)) != 0) {
        return 11;
    }

    if (expect_false(command_parse("SET TEMP_WARN 2147483648", &command)) != 0) {
        return 12;
    }

    /* 这两个边界值刚好越过 int32 范围，解析器必须拒绝而不是溢出。 */
    if (expect_false(command_parse("SET TEMP_WARN -2147483649", &command)) != 0) {
        return 13;
    }

    if (expect_false(command_parse("SAMPLE 250 500 300", &command)) != 0) {
        return 14;
    }

    if (expect_false(command_parse("SAMPLE 250 500 300 nope", &command)) != 0) {
        return 15;
    }

    if (expect_false(command_parse("SAMPLE 250 500 300 20 extra", &command)) != 0) {
        return 16;
    }

    if (expect_false(command_parse(0, &command)) != 0) {
        return 17;
    }

    if (expect_false(command_parse("STATUS?", 0)) != 0) {
        return 18;
    }

    /* name 函数输出会出现在日志/调试里，也需要固定下来。 */
    if (expect_string(command_type_name(COMMAND_TYPE_SET_THRESHOLD), "set-threshold") != 0) {
        return 19;
    }

    if (expect_string(command_type_name(COMMAND_TYPE_SET_SAMPLE), "set-sample") != 0) {
        return 20;
    }

    if (expect_string(command_threshold_name(COMMAND_THRESHOLD_SMOKE_ALARM), "smoke-alarm") != 0) {
        return 21;
    }

    return 0;
}
