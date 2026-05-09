# Serial Command Service

## Goal
Add a small app-layer service that connects byte-by-byte serial input to `serial_line` and `command_session`, then sends complete command responses through a caller-provided write callback.

## Scope
- Add app module:
  - `app/include/embedded_workbench/serial_command_service.h`
  - `app/src/serial_command_service.c`
- Add host tests:
  - `tests/test_serial_command_service.c`
- Update CMake.
- Add design and learning docs.
- Do not add a real STM32 UART driver yet.

## Steps
1. Define the service boundary and status enum.
2. Implement init/feed flow with caller-owned buffers and write callback.
3. Add tests for partial input, LF/CRLF completion, SET side effects, overflow recovery, writer failure, and invalid arguments.
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
- 2026-05-09: Started after README roadmap update and CI maintenance were merged.
- 2026-05-09: Added serial command service API, implementation, CMake target, and host tests.
- 2026-05-09: Added design and learning docs, and updated README learning indexes.
- 2026-05-09: Updated `host_sim --script` to use the serial command service path.
- 2026-05-09: Local validation passed: `git diff --check`, host build, 25 host tests, and all firmware build gates.
