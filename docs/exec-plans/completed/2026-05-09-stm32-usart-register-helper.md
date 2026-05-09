# STM32 USART Register Helper

## Goal
Add a small, host-testable STM32 USART register helper so the next UART integration step has a safe low-level boundary for baud configuration, TX/RX enable, and byte polling.

## Scope
- Add driver module:
  - `drivers/include/embedded_workbench/stm32_usart.h`
  - `drivers/src/stm32_usart.c`
- Add host tests:
  - `tests/test_stm32_usart.c`
- Update CMake.
- Add design and learning docs.
- Do not yet wire real USART2 addresses, GPIO alternate-function setup, interrupts, DMA, or `serial_command_service`.

## Steps
1. Define a minimal USART register struct and config struct.
2. Implement 8N1 oversampling-by-16 configuration and polling byte helpers.
3. Add tests for baud register calculation, CR1/CR2/CR3 field writes, invalid configs, RX ready, and TX ready.
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
- 2026-05-09: Started after serial command service was merged and main CI passed.
- 2026-05-09: Checked STM32F401 USART reference material for USART2 PA2/PA3 AF7 and core USART enable/TE/RE/BRR flow.
- 2026-05-09: Added STM32 USART helper API, implementation, CMake target, host tests, and learning/design docs.
- 2026-05-09: Local validation passed: `git diff --check`, host build, 26 host tests, and all firmware build gates.
