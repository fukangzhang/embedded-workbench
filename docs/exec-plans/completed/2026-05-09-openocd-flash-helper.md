# OpenOCD Flash Helper

## Goal
Add a small, explainable OpenOCD flashing helper for NUCLEO-F401RE so the generated firmware images have a documented path toward board bring-up.

## Scope
- Add a PowerShell helper script with a dry-run mode.
- Document the OpenOCD command shape and when to use ELF/HEX/BIN.
- Update the NUCLEO-F401RE bring-up checklist and learning index.
- Do not require OpenOCD in CI.
- Do not claim real board validation until hardware has been flashed.

## Steps
1. Add `scripts/flash_nucleo_f401re_openocd.ps1`.
2. Add learning/design notes for the OpenOCD flashing boundary.
3. Update `hardware/nucleo-f401re-bringup.md`.
4. Validate script dry-run, host tests, and firmware artifact builds.
5. Commit, push, open PR, wait for CI, and merge when green.

## Validation
- `git diff --check`
- PowerShell dry-run for the flash helper.
- `cmake --build build`
- `ctest --test-dir build --output-on-failure`
- Firmware build with artifact existence check.

## Open Questions
- Real board validation still depends on connecting NUCLEO-F401RE and installing OpenOCD locally.

## Progress Log
- 2026-05-09: Started after firmware ELF/BIN/HEX artifact PR merged and main CI passed.
- 2026-05-09: Added OpenOCD dry-run/flash helper, design note, learning note, README references, and NUCLEO-F401RE bring-up instructions.
- 2026-05-09: Validation passed: `git diff --check`, ELF dry-run, BIN dry-run, host build, 30/30 CTest, real USART2 command firmware clean rebuild, and ELF/BIN/HEX artifact existence check.
