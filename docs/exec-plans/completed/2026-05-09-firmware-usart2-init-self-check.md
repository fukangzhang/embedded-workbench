# Firmware USART2 Init Self Check

## Goal
Wire the board-level USART2 initialization into the firmware self-check path using simulated registers, so the target firmware build proves the USART2 init chain links and runs in `main`.

## Scope
- Update `firmware/src/main.c`.
- Add learning and design docs for the firmware USART2 init self-check.
- Update reading indexes.
- Do not enable real USART2 memory-mapped access yet.
- Do not connect UART bytes to `serial_command_service` yet.

## Steps
1. Add simulated APB1 and USART2 registers to `firmware/src/main.c`.
2. Add a `firmware_stm32_usart2_init_self_check` helper that initializes contexts, calls `stm32_board_usart2_init`, and checks RCC/GPIO/USART register results.
3. Include the helper in the existing `firmware_self_check` condition.
4. Document what this self-check proves and what it does not prove.
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
- 2026-05-09: Started after board-level USART2 init merged to `main` and main CI passed.
- 2026-05-09: Added firmware USART2 init self-check and learning/design docs.
- 2026-05-09: Validation passed: `git diff --check`, host build, 28/28 CTest, firmware build, real GPIO firmware build, FreeRTOS firmware build, and scheduler firmware build.
