# STM32 GPIO Alternate Function

## Goal
Extend the STM32 GPIO configuration helper so it can configure pins for alternate function mode, which is required before USART2 can use PA2/PA3 as TX/RX.

## Scope
- Extend:
  - `drivers/include/embedded_workbench/stm32_gpio_config.h`
  - `drivers/src/stm32_gpio_config.c`
  - `tests/test_stm32_gpio_config.c`
- Update docs and learning indexes.
- Do not yet bind USART2 pins or call the new API from firmware.

## Steps
1. Add AFRL/AFRH register coverage to the GPIO register struct while preserving current fields.
2. Add an alternate-function config type and `stm32_gpio_configure_alternate_function`.
3. Add tests for AF7 on low/high pins, invalid AF values, invalid pins, and unchanged-register failure paths.
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
- 2026-05-09: Started after STM32 USART register helper was merged and main CI passed.
- 2026-05-09: Added alternate-function GPIO config API, tests, and design/learning docs.
- 2026-05-09: Local validation passed: `git diff --check`, host build, 26 host tests, and all firmware build gates.
