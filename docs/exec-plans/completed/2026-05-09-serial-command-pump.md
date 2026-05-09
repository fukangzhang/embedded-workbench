# Serial Command Pump

## Goal
Add a reusable serial command pump that reads available bytes from a nonblocking reader callback and feeds them into `serial_command_service`.

## Scope
- Add:
  - `app/include/embedded_workbench/serial_command_pump.h`
  - `app/src/serial_command_pump.c`
  - `tests/test_serial_command_pump.c`
- Update CMake and docs.
- Do not wire the pump into firmware `main` yet.
- Do not add interrupts, DMA, or RTOS task scheduling.

## Steps
1. Define a reader callback type and a bounded polling function.
2. Stop polling on no data, max bytes, response sent, overflow, or error.
3. Add host tests for no data, partial line, complete command, max-byte limit, overflow, and invalid arguments.
4. Update learning/design docs and indexes.
5. Run host and firmware validation.
6. Commit, push, open PR, wait for CI, and merge when green.

## Validation
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Firmware build checks.

## Open Questions
- None.

## Progress Log
- 2026-05-09: Started after firmware USART2 command service self-check merged to `main` and main CI passed.
- 2026-05-09: Added serial command pump module, host tests, and learning/design docs.
- 2026-05-09: Validation passed: `git diff --check`, host build, 30/30 CTest, firmware build, real GPIO firmware build, FreeRTOS firmware build, and scheduler firmware build.
