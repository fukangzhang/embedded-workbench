#include "embedded_workbench/environment_processor.h"

static bool alarm_state_is_known(alarm_state_t state)
{
    return state == ALARM_STATE_NORMAL ||
           state == ALARM_STATE_WARNING ||
           state == ALARM_STATE_ALARM ||
           state == ALARM_STATE_SENSOR_FAULT;
}

bool environment_processor_init(
    environment_processor_t *processor,
    const alarm_config_t *config,
    alarm_state_t initial_state,
    environment_processor_publish_fn publish,
    void *publish_context)
{
    if (processor == 0 ||
        config == 0 ||
        !alarm_config_is_valid(config) ||
        !alarm_state_is_known(initial_state) ||
        publish == 0) {
        return false;
    }

    processor->config = *config;
    processor->state = initial_state;
    processor->publish = publish;
    processor->publish_context = publish_context;

    return true;
}

bool environment_processor_process_sample(
    environment_processor_t *processor,
    const sensor_sample_t *sample)
{
    alarm_state_t next_state;

    if (processor == 0 ||
        processor->publish == 0 ||
        !alarm_config_is_valid(&processor->config) ||
        !sensor_sample_is_valid(sample)) {
        return false;
    }

    next_state = alarm_state_update(processor->state, &processor->config, sample);

    if (!processor->publish(processor->publish_context, next_state)) {
        return false;
    }

    processor->state = next_state;
    return true;
}

bool environment_processor_update_config(
    environment_processor_t *processor,
    const alarm_config_t *config)
{
    if (processor == 0 || !alarm_config_is_valid(config)) {
        return false;
    }

    processor->config = *config;
    return true;
}

alarm_state_t environment_processor_current_state(const environment_processor_t *processor)
{
    if (processor == 0) {
        return ALARM_STATE_SENSOR_FAULT;
    }

    return processor->state;
}
