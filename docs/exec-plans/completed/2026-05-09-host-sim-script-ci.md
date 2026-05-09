# host_sim Script CI Portability

## Goal
Fix PR #42 so the `host_sim_script_status` test runs on both Windows local builds and Ubuntu GitHub Actions.

## Scope
- Replace the PowerShell-only script test runner with a CMake script runner.
- Keep the existing `host_sim --script` behavior under test.
- Do not change host simulator command semantics.

## Steps
1. Register the script-mode smoke test through `${CMAKE_COMMAND} -P`.
2. Add a small CMake test script that executes `host_sim --script` with a text input file.
3. Validate the host test locally, then rerun full build/test gates and PR checks.

## Validation
- `cmake --build build`
- `ctest --test-dir build --output-on-failure -R host_sim_script_status`
- `ctest --test-dir build --output-on-failure`
- Firmware build checks if host validation passes.

## Open Questions
- None.

## Progress Log
- 2026-05-09: Started after GitHub Actions failed because Ubuntu could not find `powershell`.
- 2026-05-09: Replaced the PowerShell smoke test with a CMake script and text input file.
- 2026-05-09: Local validation passed for the focused script test, full host tests, and firmware build gates.
