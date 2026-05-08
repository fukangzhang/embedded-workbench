#include <stdbool.h>
#include <string.h>

#include "embedded_workbench/digital_output.h"

typedef struct {
    int write_calls;
    const board_pin_t *last_pin;
    digital_output_level_t last_level;
    bool fail_write;
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

static int expect_string(const char *actual, const char *expected)
{
    return strcmp(actual, expected) == 0 ? 0 : 1;
}

static bool fake_write(
    void *context,
    const board_pin_t *pin,
    digital_output_level_t level)
{
    fake_digital_output_context_t *fake = (fake_digital_output_context_t *)context;

    fake->write_calls++;
    fake->last_pin = pin;
    fake->last_level = level;

    return !fake->fail_write;
}

int main(void)
{
    static const board_pin_t pin = {"PA", 5u, "test output"};
    static const digital_output_ops_t ops = {
        fake_write,
    };
    fake_digital_output_context_t fake = {0};
    digital_output_controller_t controller = {&ops, &fake};
    board_pin_t invalid_pin = {0};

    if (expect_true(digital_output_write(&controller, &pin, DIGITAL_OUTPUT_LEVEL_HIGH)) != 0 ||
        expect_int(fake.write_calls, 1) != 0 ||
        expect_ptr(fake.last_pin, &pin) != 0 ||
        expect_level(fake.last_level, DIGITAL_OUTPUT_LEVEL_HIGH) != 0) {
        return 1;
    }

    if (expect_false(digital_output_write(0, &pin, DIGITAL_OUTPUT_LEVEL_LOW)) != 0 ||
        expect_false(digital_output_write(&controller, 0, DIGITAL_OUTPUT_LEVEL_LOW)) != 0 ||
        expect_false(digital_output_write(&controller, &invalid_pin, DIGITAL_OUTPUT_LEVEL_LOW)) != 0 ||
        expect_false(digital_output_write(&controller, &pin, (digital_output_level_t)99)) != 0) {
        return 2;
    }

    fake.fail_write = true;
    if (expect_false(digital_output_write(&controller, &pin, DIGITAL_OUTPUT_LEVEL_LOW)) != 0 ||
        expect_int(fake.write_calls, 2) != 0 ||
        expect_level(fake.last_level, DIGITAL_OUTPUT_LEVEL_LOW) != 0) {
        return 3;
    }

    if (expect_level(digital_output_level_from_bool(true), DIGITAL_OUTPUT_LEVEL_HIGH) != 0 ||
        expect_level(digital_output_level_from_bool(false), DIGITAL_OUTPUT_LEVEL_LOW) != 0) {
        return 4;
    }

    if (expect_string(digital_output_level_name(DIGITAL_OUTPUT_LEVEL_HIGH), "high") != 0 ||
        expect_string(digital_output_level_name((digital_output_level_t)99), "unknown") != 0) {
        return 5;
    }

    return 0;
}
