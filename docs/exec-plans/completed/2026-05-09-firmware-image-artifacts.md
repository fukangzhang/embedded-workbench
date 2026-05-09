# Firmware Image Artifacts

## Goal
Generate flashable `.bin` and `.hex` images from `embedded_firmware.elf` so board bring-up has concrete firmware artifacts to flash.

## Scope
- Update `CMakeLists.txt`.
- Update hardware and learning docs.
- Do not add a flashing tool wrapper yet.
- Do not assume a specific programmer is installed.

## Steps
1. Add post-build objcopy commands for firmware `.bin` and `.hex`.
2. Support both GNU objcopy and Zig objcopy paths.
3. Verify normal firmware, real GPIO firmware, and real USART2 command firmware produce artifacts.
4. Update bring-up docs and learning index.
5. Run host and firmware validation.
6. Commit, push, open PR, wait for CI, and merge when green.

## Validation
- `git diff --check`
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Firmware build checks, including artifact existence.

## Open Questions
- None.

## Progress Log
- 2026-05-09: Started after real USART2 command loop switch merged to `main` and main CI passed.
- 2026-05-09: Added firmware `.bin/.hex` post-build generation and docs; Zig objcopy uses `-O hex`.
- 2026-05-09: Validation passed: `git diff --check`, host build, 30/30 CTest, default firmware build, real GPIO firmware build, real USART2 command loop firmware build, FreeRTOS firmware build, scheduler firmware build, and ELF/BIN/HEX artifact existence checks.
