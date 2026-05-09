# Firmware USART2 Command Pump Self Check

## Goal
Use the new `serial_command_pump` inside the firmware USART2 command-service self-check, so firmware follows the same bounded reader-to-service path planned for real USART2 polling.

## Scope
- Update `firmware/src/main.c`.
- Update learning/design docs and indexes.
- Do not enable a real infinite UART command loop yet.
- Do not add interrupts, DMA, RTOS task scheduling, or real USART2 RX polling.

## Steps
1. Include `serial_command_pump` in firmware.
2. Add a tiny text reader callback for the firmware self-check input.
3. Replace the manual `while` feed loop with `serial_command_pump_poll`.
4. Update docs to explain the self-check now covers the pump path.
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
- 2026-05-09: Started after serial command pump merged to `main` and main CI passed.
- 2026-05-09: Replaced the firmware USART2 command-service manual feed loop with `serial_command_pump_poll` and updated docs.
- 2026-05-09: Validation passed: `git diff --check`, host build, 30/30 CTest, firmware build, real GPIO firmware build, FreeRTOS firmware build, and scheduler firmware build.
