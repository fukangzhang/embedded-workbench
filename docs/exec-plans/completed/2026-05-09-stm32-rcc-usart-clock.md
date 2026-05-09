# STM32 RCC USART Clock

## Goal
Add a host-testable RCC helper for enabling STM32 USART peripheral clocks, preparing for a USART2 board-level initialization path.

## Scope
- Add:
  - `drivers/include/embedded_workbench/stm32_rcc_usart_clock.h`
  - `drivers/src/stm32_rcc_usart_clock.c`
  - `tests/test_stm32_rcc_usart_clock.c`
- Update CMake and docs.
- Do not yet bind STM32F401RE APB1ENR addresses or initialize USART2.

## Steps
1. Mirror the GPIO RCC clock helper style for USART peripheral names.
2. Add tests for enable bits, idempotence, unknown peripherals, invalid bits, and invalid init arguments.
3. Update learning/design docs and indexes.
4. Run host and firmware validation.
5. Commit, push, open PR, wait for CI, and merge when green.

## Validation
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Firmware build checks.

## Open Questions
- None.

## Progress Log
- 2026-05-09: Started after GPIO alternate function support was merged and main CI passed.
- 2026-05-09: Added RCC USART clock helper, tests, docs, and indexes.
- 2026-05-09: Local validation passed: `git diff --check`, host build, 27 host tests, and all firmware build gates.
