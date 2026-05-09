# STM32F401RE USART2 Bindings

## Goal
Expose STM32F401RE USART2 address and board facts so the next step can initialize a real USART2 path using existing RCC, GPIO AF, and USART helpers.

## Scope
- Extend:
  - `drivers/include/embedded_workbench/stm32f401re_gpio_bindings.h`
  - `drivers/src/stm32f401re_gpio_bindings.c`
  - `tests/test_stm32f401re_gpio_bindings.c`
- Add USART2:
  - RCC APB1ENR address
  - USART2 base address
  - USART2 APB1 enable bit
  - PA2 TX / PA3 RX pins
  - AF7 number
- Update docs.
- Do not initialize USART2 yet.

## Steps
1. Add constants and accessor functions.
2. Extend binding tests to compare address values and static table contents without dereferencing target addresses.
3. Update design/learning docs.
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
- 2026-05-09: Started after RCC USART clock helper was merged and main CI passed.
- 2026-05-09: Added STM32F401RE USART2 address, RCC, PA2/PA3, AF7 bindings, tests, and docs.
- 2026-05-09: Local validation passed: `git diff --check`, host build, 27 host tests, and all firmware build gates.
