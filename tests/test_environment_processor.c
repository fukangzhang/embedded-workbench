#include <stdbool.h>

#include "embedded_workbench/environment_processor.h"

typedef struct {
    unsigned int count;
    alarm_state_t last_state;
    bool fail_next_publish;
} fake_publish_context_t;

static bool fake_publish(void *context, alarm_state_t state)
{
    fake_publish_context_t *fake = (fake_publish_context_t *)context;

    if (fake == 0 || fake->fail_next_publish) {
        return false;
    }

    fake->last_state = state;
    fake->count++;

    return true;
}

static bool null_context_publish(void *context, alarm_state_t state)
{
    (void)context;

    return state == ALARM_STATE_NORMAL ||
           state == ALARM_STATE_WARNING ||
           state == ALARM_STATE_ALARM ||
           state == ALARM_STATE_SENSOR_FAULT;
}

static int expect_true(bool value)
{
    return value ? 0 : 1;
}

static int expect_false(bool value)
{
    return value ? 1 : 0;
}

static int expect_uint(unsigned int actual, unsigned int expected)
{
    return actual == expected ? 0 : 1;
}

static int expect_state(alarm_state_t actual, alarm_state_t expected)
{
    return actual == expected ? 0 : 1;
}

static int test_update_config_changes_next_processing_result(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_config_t updated_config = config;
    alarm_config_t invalid_config = config;
    sensor_sample_t sample = sensor_sample_make(340, 500u, 300u, 20u);
    environment_processor_t processor;
    fake_publish_context_t publish_context = {0u, ALARM_STATE_SENSOR_FAULT, false};

    if (expect_true(environment_processor_init(
            &processor,
            &config,
            ALARM_STATE_NORMAL,
            fake_publish,
            &publish_context)) != 0) {
        return 1;
    }

    if (expect_true(environment_processor_process_sample(&processor, &sample)) != 0 ||
        expect_state(publish_context.last_state, ALARM_STATE_NORMAL) != 0) {
        return 2;
    }

    updated_config.temperature_warning_high_c_x10 = 330;
    if (expect_true(environment_processor_update_config(&processor, &updated_config)) != 0 ||
        expect_true(environment_processor_process_sample(&processor, &sample)) != 0 ||
        expect_state(publish_context.last_state, ALARM_STATE_WARNING) != 0 ||
        expect_state(environment_processor_current_state(&processor), ALARM_STATE_WARNING) != 0) {
        return 3;
    }

    invalid_config.temperature_warning_high_c_x10 = 500;
    invalid_config.temperature_alarm_high_c_x10 = 450;
    sample = sensor_sample_make(460, 500u, 300u, 20u);

    if (expect_false(environment_processor_update_config(&processor, &invalid_config)) != 0 ||
        expect_true(environment_processor_process_sample(&processor, &sample)) != 0 ||
        expect_state(publish_context.last_state, ALARM_STATE_ALARM) != 0) {
        return 4;
    }

    if (expect_false(environment_processor_update_config(0, &config)) != 0 ||
        expect_false(environment_processor_update_config(&processor, 0)) != 0) {
        return 5;
    }

    return 0;
}

int main(void)
{
    alarm_config_t config = alarm_config_default();
    alarm_config_t invalid_config = config;
    sensor_sample_t normal_sample = sensor_sample_make(250, 500u, 300u, 20u);
    sensor_sample_t warning_sample = sensor_sample_make(360, 500u, 300u, 20u);
    sensor_sample_t alarm_sample = sensor_sample_make(460, 500u, 300u, 20u);
    sensor_sample_t invalid_sample = sensor_sample_make(SENSOR_TEMPERATURE_MAX_C_X10 + 1, 500u, 300u, 20u);
    environment_processor_t processor;
    fake_publish_context_t publish_context = {0u, ALARM_STATE_SENSOR_FAULT, false};

    if (expect_true(environment_processor_init(
            &processor,
            &config,
            ALARM_STATE_NORMAL,
            fake_publish,
            &publish_context)) != 0) {
        return 1;
    }

    if (expect_true(environment_processor_process_sample(&processor, &normal_sample)) != 0 ||
        expect_uint(publish_context.count, 1u) != 0 ||
        expect_state(publish_context.last_state, ALARM_STATE_NORMAL) != 0 ||
        expect_state(environment_processor_current_state(&processor), ALARM_STATE_NORMAL) != 0) {
        return 2;
    }

    if (expect_true(environment_processor_process_sample(&processor, &warning_sample)) != 0 ||
        expect_uint(publish_context.count, 2u) != 0 ||
        expect_state(publish_context.last_state, ALARM_STATE_WARNING) != 0 ||
        expect_state(environment_processor_current_state(&processor), ALARM_STATE_WARNING) != 0) {
        return 3;
    }

    if (expect_true(environment_processor_process_sample(&processor, &alarm_sample)) != 0 ||
        expect_uint(publish_context.count, 3u) != 0 ||
        expect_state(publish_context.last_state, ALARM_STATE_ALARM) != 0 ||
        expect_state(environment_processor_current_state(&processor), ALARM_STATE_ALARM) != 0) {
        return 4;
    }

    publish_context.fail_next_publish = true;
    if (expect_false(environment_processor_process_sample(&processor, &normal_sample)) != 0 ||
        expect_uint(publish_context.count, 3u) != 0 ||
        expect_state(environment_processor_current_state(&processor), ALARM_STATE_ALARM) != 0) {
        return 5;
    }
    publish_context.fail_next_publish = false;

    if (expect_false(environment_processor_process_sample(&processor, &invalid_sample)) != 0 ||
        expect_uint(publish_context.count, 3u) != 0 ||
        expect_state(environment_processor_current_state(&processor), ALARM_STATE_ALARM) != 0) {
        return 6;
    }

    invalid_config.temperature_warning_high_c_x10 = 500;
    invalid_config.temperature_alarm_high_c_x10 = 450;
    if (expect_false(environment_processor_init(
            &processor,
            &invalid_config,
            ALARM_STATE_NORMAL,
            fake_publish,
            &publish_context)) != 0) {
        return 7;
    }

    if (expect_false(environment_processor_init(0, &config, ALARM_STATE_NORMAL, fake_publish, &publish_context)) != 0 ||
        expect_false(environment_processor_init(&processor, 0, ALARM_STATE_NORMAL, fake_publish, &publish_context)) != 0 ||
        expect_false(environment_processor_init(&processor, &config, (alarm_state_t)99, fake_publish, &publish_context)) != 0 ||
        expect_false(environment_processor_init(&processor, &config, ALARM_STATE_NORMAL, 0, &publish_context)) != 0 ||
        expect_false(environment_processor_process_sample(0, &normal_sample)) != 0 ||
        expect_state(environment_processor_current_state(0), ALARM_STATE_SENSOR_FAULT) != 0) {
        return 8;
    }

    if (expect_true(environment_processor_init(
            &processor,
            &config,
            ALARM_STATE_NORMAL,
            null_context_publish,
            0)) != 0 ||
        expect_true(environment_processor_process_sample(&processor, &normal_sample)) != 0) {
        return 9;
    }

    if (test_update_config_changes_next_processing_result() != 0) {
        return 10;
    }

    return 0;
}
