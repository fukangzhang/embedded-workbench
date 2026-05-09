#include <stdbool.h>

#include "embedded_workbench/sensor_sample.h"

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

int main(void)
{
    /* 这个测试先构造一组代表性样本：
     * normal 是普通有效值，min/max 验证边界值刚好可接受。 */
    sensor_sample_t normal = sensor_sample_make(253, 604, 350u, 12u);
    sensor_sample_t min_values = sensor_sample_make(
        SENSOR_TEMPERATURE_MIN_C_X10,
        SENSOR_HUMIDITY_MIN_RH_X10,
        0u,
        0u);
    sensor_sample_t max_values = sensor_sample_make(
        SENSOR_TEMPERATURE_MAX_C_X10,
        SENSOR_HUMIDITY_MAX_RH_X10,
        SENSOR_LIGHT_MAX_LUX,
        SENSOR_SMOKE_MAX_PPM);
    sensor_sample_t too_cold = sensor_sample_make(
        SENSOR_TEMPERATURE_MIN_C_X10 - 1,
        500u,
        100u,
        10u);
    sensor_sample_t too_hot = sensor_sample_make(
        SENSOR_TEMPERATURE_MAX_C_X10 + 1,
        500u,
        100u,
        10u);
    sensor_sample_t too_humid = sensor_sample_make(250, SENSOR_HUMIDITY_MAX_RH_X10 + 1, 100u, 10u);
    sensor_sample_t too_bright = sensor_sample_make(250, 500u, SENSOR_LIGHT_MAX_LUX + 1u, 10u);
    sensor_sample_t too_smoky = sensor_sample_make(250, 500u, 100u, SENSOR_SMOKE_MAX_PPM + 1u);

    /* 传感器模型的核心契约：合法范围内的值返回 true。 */
    if (expect_true(sensor_sample_is_valid(&normal)) != 0) {
        return 1;
    }

    if (expect_true(sensor_sample_is_valid(&min_values)) != 0) {
        return 2;
    }

    if (expect_true(sensor_sample_is_valid(&max_values)) != 0) {
        return 3;
    }

    /* 每个字段各测一个越界值，能定位是哪类输入边界被破坏。 */
    if (expect_false(sensor_sample_is_valid(&too_cold)) != 0) {
        return 4;
    }

    if (expect_false(sensor_sample_is_valid(&too_hot)) != 0) {
        return 5;
    }

    if (expect_false(sensor_sample_is_valid(&too_humid)) != 0) {
        return 6;
    }

    if (expect_false(sensor_sample_is_valid(&too_bright)) != 0) {
        return 7;
    }

    if (expect_false(sensor_sample_is_valid(&too_smoky)) != 0) {
        return 8;
    }

    /* C 里空指针是常见错误输入；公共校验函数必须安全拒绝。 */
    if (expect_false(sensor_sample_is_valid(0)) != 0) {
        return 9;
    }

    return 0;
}
