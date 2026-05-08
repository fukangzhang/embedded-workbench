#include <stdbool.h>

#include "embedded_workbench/alarm_output_digital_sink.h"

typedef struct {
    int write_calls;
    int fail_at_call;
    const board_pin_t *pins[6];
    digital_output_level_t levels[6];
} fake_digital_output_context_t;

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

static int expect_ptr(const void *actual, const void *expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_level(digital_output_level_t actual, digital_output_level_t expected)
{
    return actual == expected ? 0 : 1;
}

static bool fake_write(
    void *context,
    const board_pin_t *pin,
    digital_output_level_t level)
{
    fake_digital_output_context_t *fake = (fake_digital_output_context_t *)context;
    int call_index = fake->write_calls;

    if (call_index >= 0 && call_index < 6) {
        fake->pins[call_index] = pin;
        fake->levels[call_index] = level;
    }

    fake->write_calls++;

    return fake->fail_at_call == 0 || fake->write_calls != fake->fail_at_call;
}

static void reset_fake(fake_digital_output_context_t *fake)
{
    int index = 0;

    fake->write_calls = 0;
    fake->fail_at_call = 0;
    for (index = 0; index < 6; index++) {
        fake->pins[index] = 0;
        fake->levels[index] = DIGITAL_OUTPUT_LEVEL_LOW;
    }
}

int main(void)
{
    static const digital_output_ops_t digital_ops = {
        fake_write,
    };
    const board_profile_t *profile = board_profile_default();
    fake_digital_output_context_t fake = {0};
    digital_output_controller_t digital_output = {&digital_ops, &fake};
    alarm_output_digital_sink_context_t digital_sink_context;
    alarm_output_sink_t sink;
    alarm_output_command_t command;

    if (expect_true(alarm_output_digital_sink_init(
            &sink,
            &digital_sink_context,
            &digital_output,
            profile)) != 0) {
        return 1;
    }

    command.indicator = ALARM_OUTPUT_INDICATOR_FAST_BLINK;
    command.buzzer_enabled = true;
    command.actuator_enabled = true;
    command.period_ms = 250u;

    if (expect_true(alarm_output_sink_apply(&sink, &command)) != 0 ||
        expect_int(fake.write_calls, 3) != 0 ||
        expect_ptr(fake.pins[0], &profile->alarm_led) != 0 ||
        expect_level(fake.levels[0], DIGITAL_OUTPUT_LEVEL_HIGH) != 0 ||
        expect_ptr(fake.pins[1], &profile->alarm_buzzer) != 0 ||
        expect_level(fake.levels[1], DIGITAL_OUTPUT_LEVEL_HIGH) != 0 ||
        expect_ptr(fake.pins[2], &profile->alarm_actuator) != 0 ||
        expect_level(fake.levels[2], DIGITAL_OUTPUT_LEVEL_HIGH) != 0) {
        return 2;
    }

    reset_fake(&fake);
    command.indicator = ALARM_OUTPUT_INDICATOR_OFF;
    command.buzzer_enabled = false;
    command.actuator_enabled = false;
    command.period_ms = 0u;

    if (expect_true(alarm_output_sink_apply(&sink, &command)) != 0 ||
        expect_int(fake.write_calls, 3) != 0 ||
        expect_level(fake.levels[0], DIGITAL_OUTPUT_LEVEL_LOW) != 0 ||
        expect_level(fake.levels[1], DIGITAL_OUTPUT_LEVEL_LOW) != 0 ||
        expect_level(fake.levels[2], DIGITAL_OUTPUT_LEVEL_LOW) != 0) {
        return 3;
    }

    reset_fake(&fake);
    fake.fail_at_call = 2;
    command.indicator = ALARM_OUTPUT_INDICATOR_FAST_BLINK;
    command.buzzer_enabled = true;
    command.actuator_enabled = true;

    if (expect_false(alarm_output_sink_apply(&sink, &command)) != 0 ||
        expect_int(fake.write_calls, 2) != 0) {
        return 4;
    }

    if (expect_false(alarm_output_digital_sink_init(0, &digital_sink_context, &digital_output, profile)) != 0 ||
        expect_false(alarm_output_digital_sink_init(&sink, 0, &digital_output, profile)) != 0 ||
        expect_false(alarm_output_digital_sink_init(&sink, &digital_sink_context, 0, profile)) != 0 ||
        expect_false(alarm_output_digital_sink_init(&sink, &digital_sink_context, &digital_output, 0)) != 0) {
        return 5;
    }

    return 0;
}
