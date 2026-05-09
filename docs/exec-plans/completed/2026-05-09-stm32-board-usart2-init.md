# STM32 Board USART2 Init

## Goal
Add a board-level USART2 initialization function that wires the existing GPIO clock, GPIO alternate function, USART clock, USART register helper, and STM32F401RE bindings together.

## Scope
- Add:
  - `drivers/include/embedded_workbench/stm32_board_usart2_init.h`
  - `drivers/src/stm32_board_usart2_init.c`
  - `tests/test_stm32_board_usart2_init.c`
- Update CMake and docs.
- Do not yet call this from firmware `main` or connect TX/RX to `serial_command_service`.

## Steps
1. Define a small init API that receives already-created contexts and USART config.
2. Implement the ordered init sequence: GPIOA clock, PA2/PA3 AF7, USART2 clock, USART 8N1 config.
3. Add host tests using fake registers.
4. Update docs.
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
- 2026-05-09: Started after STM32F401RE USART2 bindings were merged and main CI passed.
- 2026-05-09: Added board-level USART2 init API, implementation, host test, and learning/design docs.
- 2026-05-09: Validation passed: `git diff --check`, host build, 28/28 CTest, firmware build, real GPIO firmware build, FreeRTOS firmware build, and scheduler firmware build.
