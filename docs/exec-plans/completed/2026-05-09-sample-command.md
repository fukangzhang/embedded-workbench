# Sample Injection Command

## Goal
Add a serial/host command that updates the current sensor sample so demos can drive alarm states before real sensors are connected.

## Scope
- Add `SAMPLE <temp_c_x10> <humidity_rh_x10> <light_lux> <smoke_ppm>` to the text command protocol.
- Validate sample ranges with existing `sensor_sample_is_valid` rules.
- Recompute alarm state after a valid sample update and return a fresh status response.
- Update tests and learning docs.
- Do not add real sensor drivers in this task.

## Steps
1. Extend `command_parser` syntax and tests.
2. Extend `command_handler` to validate and commit sample updates.
3. Update `command_session` to hold a mutable sample and append status after sample changes.
4. Update host/script smoke coverage and docs.
5. Run host tests and firmware builds.
6. Commit, push, open PR, wait for CI, and merge when green.

## Validation
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Firmware build checks for normal and real USART2 command loop builds.

## Open Questions
- Real sensor acquisition remains a later task; this command is only a controlled demo/test input.

## Progress Log
- 2026-05-09: Started from clean `main` after OpenOCD helper merged and main CI passed.
- 2026-05-09: Added `SAMPLE` command parsing, handler validation, session sample update/status response, serial service coverage, host script smoke coverage, and learning/design docs.
- 2026-05-09: Validation passed: `git diff --check`, host build, 30/30 CTest, default firmware clean build, real USART2 command firmware clean build, FreeRTOS firmware clean build, scheduler firmware clean build, and firmware artifact existence checks.
