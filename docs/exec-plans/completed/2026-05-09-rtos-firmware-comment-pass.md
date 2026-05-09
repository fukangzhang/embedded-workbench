# C Code Comment Pass - RTOS Firmware

## Goal
Add beginner-friendly comments to the RTOS abstraction and firmware skeleton, so a C/embedded beginner can understand how task models, queues, FreeRTOS glue, startup code, and firmware self-checks fit together.

## Scope
- Cover:
  - `app/include/embedded_workbench/rtos_task_model.h`
  - `app/src/rtos_task_model.c`
  - `app/include/embedded_workbench/rtos_port.h`
  - `app/src/rtos_port.c`
  - `app/include/embedded_workbench/rtos_port_freertos.h`
  - `app/src/rtos_port_freertos.c`
  - `firmware/src/main.c`
  - `firmware/startup/startup_stm32f401re.c`
- Explain core logic in detail: task/queue descriptors, operation-table interfaces, queue handoff, scheduler start behavior, firmware self-checks, vector table, reset handler, and default handlers.
- Keep repetitive descriptor tables and simple wrappers lightly commented.
- Do not change behavior or refactor code.

## Steps
1. Read current RTOS/firmware code and existing learning notes.
2. Add module-level comments plus focused inline comments for core embedded concepts.
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
- 2026-05-09: Started after PR #44 was merged and main CI passed.
- 2026-05-09: Added beginner-focused comments to RTOS model/port files, firmware main, and startup code.
- 2026-05-09: Updated the C comment reading note with the RTOS/firmware reading order.
- 2026-05-09: Local validation passed: `git diff --check`, host build, 24 host tests, and all firmware build gates.
