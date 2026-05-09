# Firmware Real USART2 Command Loop

## Goal
Add a default-off firmware build switch for a real STM32F401RE USART2 command polling loop, using the existing board USART2 init, USART serial IO, serial command pump, and serial command service.

## Scope
- Update `CMakeLists.txt`.
- Update `.github/workflows/ci.yml`.
- Update `firmware/src/main.c`.
- Add learning/design docs and indexes.
- Do not enable the real USART2 command loop by default.
- Do not add interrupts, DMA, ring buffers, or FreeRTOS task scheduling.

## Steps
1. Add `EW_FIRMWARE_USE_REAL_STM32_USART2_COMMAND_LOOP`.
2. Initialize real STM32F401RE USART2 command-service state when the switch is enabled.
3. Poll `serial_command_pump_poll` from the firmware idle loop when enabled and initialized.
4. Add CI build coverage for the new compile switch.
5. Update docs and indexes.
6. Run host and firmware validation.
7. Commit, push, open PR, wait for CI, and merge when green.

## Validation
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Firmware build checks, including the new real USART2 command loop build.

## Open Questions
- None.

## Progress Log
- 2026-05-09: Started after firmware command pump self-check merged to `main` and main CI passed.
- 2026-05-09: Added default-off real USART2 command loop switch, CI build coverage, firmware wiring, and docs.
- 2026-05-09: Validation passed: `git diff --check`, host build, 30/30 CTest, firmware build, real GPIO firmware build, real USART2 command loop firmware build, FreeRTOS firmware build, and scheduler firmware build.
