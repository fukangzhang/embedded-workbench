# Firmware USART2 Command Self Check

## Goal
Wire the STM32 USART serial IO adapter into the firmware self-check path through `serial_command_service`, using simulated registers.

## Scope
- Update `firmware/src/main.c`.
- Add learning and design docs.
- Update reading indexes.
- Do not enable a real infinite UART command loop yet.
- Do not enable USART2 real memory-mapped access by default.
- Do not add interrupts, DMA, or ring buffers.

## Steps
1. Add firmware-side serial command service buffers and USART serial IO context usage.
2. Feed `STATUS?\n` through `serial_command_service_feed` with `stm32_usart_serial_io_write` as the writer.
3. Check that the command response is generated and written through the fake USART2 registers.
4. Update docs and indexes.
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
- 2026-05-09: Started after STM32 USART serial IO adapter merged to `main` and main CI passed.
- 2026-05-09: Added firmware USART2 command service self-check and learning/design docs.
- 2026-05-09: Validation passed: `git diff --check`, host build, 29/29 CTest, firmware build, real GPIO firmware build, FreeRTOS firmware build, and scheduler firmware build.
