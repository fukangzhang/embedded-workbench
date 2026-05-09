# Sequence Sensor Source

## Goal
Add a reusable sequence-backed sensor source so host simulation and future tests can read samples through the same `sensor_source_t` interface used by FreeRTOS acquisition.

## Scope
- Add `sequence_sensor_source_t` in the drivers layer.
- Support finite sequence reads and optional repeat-last behavior.
- Update `host_sim` demo samples to read through the source interface.
- Add unit tests and docs.
- Do not add a real hardware sensor driver in this task.

## Steps
1. Add sequence source header/source and tests.
2. Wire it into CMake.
3. Refactor `host_sim` demo to use `sensor_source_read`.
4. Update learning/design docs and README.
5. Run host tests and firmware builds.
6. Commit, push, open PR, wait for CI, and merge when green.

## Validation
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Firmware build checks.

## Open Questions
- None for this task.

## Progress Log
- 2026-05-09: Started from clean `main` after sensor source interface merged and main CI passed.
- 2026-05-09: Added `sequence_sensor_source`, unit tests, CMake wiring, and refactored `host_sim` demo samples to read via `sensor_source_read`.
- 2026-05-09: Updated learning/design docs and README to place sequence source in the sensor input path.
- 2026-05-09: Validation passed: `git diff --check`, host build, 32/32 CTest, default firmware clean build, real USART2 command firmware clean build, FreeRTOS firmware clean build, scheduler firmware clean build, and key firmware artifact existence checks.
