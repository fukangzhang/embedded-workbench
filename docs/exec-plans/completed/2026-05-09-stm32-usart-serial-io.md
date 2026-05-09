# STM32 USART Serial IO

## Goal
Add a small adapter that turns the existing STM32 USART byte helpers into reusable serial read/write callbacks for later connection to `serial_command_service`.

## Scope
- Add:
  - `drivers/include/embedded_workbench/stm32_usart_serial_io.h`
  - `drivers/src/stm32_usart_serial_io.c`
  - `tests/test_stm32_usart_serial_io.c`
- Update CMake and docs.
- Do not yet wire this into firmware `main`.
- Do not add interrupt, DMA, or ring-buffer behavior.

## Steps
1. Define a minimal context around `stm32_usart_registers_t`.
2. Implement nonblocking read/write wrappers around `stm32_usart_read_byte` and `stm32_usart_write_byte`.
3. Add host tests with fake USART registers.
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
- 2026-05-09: Started after firmware USART2 init self-check merged to `main` and main CI passed.
- 2026-05-09: Added STM32 USART serial IO adapter, host tests, and learning/design docs.
- 2026-05-09: Validation passed: `git diff --check`, host build, 29/29 CTest, firmware build, real GPIO firmware build, FreeRTOS firmware build, and scheduler firmware build.
