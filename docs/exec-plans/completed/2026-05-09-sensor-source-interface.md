# Sensor Source Interface

## Goal
Introduce a small sensor source abstraction so sample acquisition can move from manual `SAMPLE` injection toward simulated and real sensor inputs.

## Scope
- Add `sensor_source_t` with a read callback.
- Validate that read samples satisfy existing `sensor_sample_is_valid` rules before callers accept them.
- Add a host-side unit test with a fake source.
- Wire the FreeRTOS sensor acquire task to read from an optional source and send to `sensor_sample_queue`.
- Update docs and learning notes.
- Do not implement a real I2C/SPI/ADC sensor driver in this task.

## Steps
1. Add driver interface and tests for `sensor_source`.
2. Add the new driver/test to CMake.
3. Extend FreeRTOS context with optional source and use it in the acquire task.
4. Update learning/design docs and README.
5. Run host tests and firmware builds.
6. Commit, push, open PR, wait for CI, and merge when green.

## Validation
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Firmware build checks, including FreeRTOS and scheduler builds.

## Open Questions
- Which physical sensor should be implemented first remains a later hardware task.

## Progress Log
- 2026-05-09: Started from clean `main` after `SAMPLE` command merged and main CI passed.
- 2026-05-09: Added `sensor_source_t`, host fake-source tests, CMake wiring, and optional FreeRTOS sensor acquire source-to-queue flow.
- 2026-05-09: Updated README, learning index, FreeRTOS notes, and design boundary docs.
- 2026-05-09: Validation passed: `git diff --check`, host build, 31/31 CTest, default firmware clean build, real USART2 command firmware clean build, FreeRTOS firmware clean build, scheduler firmware clean build, and key firmware artifact existence checks.
