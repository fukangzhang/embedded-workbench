# C Code Comment Pass - STM32 BSP

## Goal
Add beginner-friendly comments to the BSP and STM32 GPIO driver layer, so a C/embedded beginner can understand the boundary between board mapping, generic output abstraction, host simulation, and real STM32 register access.

## Scope
- Cover:
  - `bsp/include/embedded_workbench/board_profile.h`
  - `bsp/src/board_profile.c`
  - `drivers/include/embedded_workbench/digital_output.h`
  - `drivers/src/digital_output.c`
  - `drivers/include/embedded_workbench/board_digital_output.h`
  - `drivers/src/board_digital_output.c`
  - `drivers/include/embedded_workbench/stm32_gpio_config.h`
  - `drivers/src/stm32_gpio_config.c`
  - `drivers/include/embedded_workbench/stm32_gpio_output.h`
  - `drivers/src/stm32_gpio_output.c`
  - `drivers/include/embedded_workbench/stm32_rcc_gpio_clock.h`
  - `drivers/src/stm32_rcc_gpio_clock.c`
  - `drivers/include/embedded_workbench/stm32_board_gpio_init.h`
  - `drivers/src/stm32_board_gpio_init.c`
  - `drivers/include/embedded_workbench/stm32f401re_gpio_bindings.h`
  - `drivers/src/stm32f401re_gpio_bindings.c`
- Explain core logic in detail: board pin mapping, function-pointer output abstraction, host-backed fake output, STM32 bit fields, BSRR atomic writes, RCC clock enable bits, and real address bindings.
- Keep obvious getters and repetitive tables lightly commented.
- Do not change behavior or refactor code.

## Steps
1. Read current BSP/STM32 driver code and existing learning notes.
2. Add module-level comments plus focused inline comments for hardware boundary concepts.
3. Update the C comment reading note with this batch's reading order.
4. Run whitespace/build/test gates.
5. Commit, push, open PR, wait for CI, and merge when green.

## Validation
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Firmware build checks.

## Open Questions
- None.

## Progress Log
- 2026-05-09: Started after PR #45 was merged and main CI passed.
- 2026-05-09: Added beginner-focused comments to BSP, digital output, STM32 GPIO/RCC, board init, and F401RE binding files.
- 2026-05-09: Updated the C comment reading note with the STM32/BSP reading order.
- 2026-05-09: Local validation passed: `git diff --check`, host build, 24 host tests, and all firmware build gates.
