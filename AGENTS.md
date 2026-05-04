# AGENTS.md

## Project
- Name: Embedded Workbench
- Goal: Build a job-ready embedded software project around STM32/MCU, FreeRTOS, communication, sensors, and control.
- Current phase: bootstrap
- Primary language: C
- Build system: CMake

## Architecture
- See `ARCHITECTURE.md` for module boundaries and dependency rules.
- Preferred flow: BSP -> Drivers -> App.
- Tests and simulation code live under `tests/`.
- Documentation and plans live under `docs/`.

## Commands
- `cmake -S . -B build` - configure the project
- `cmake --build build` - build the project
- `ctest --test-dir build --output-on-failure` - run tests after they exist
- `git status --short --branch` - inspect branch and worktree

## Conventions
- One task at a time. Do not mix refactor, feature, and docs in one change.
- Prefer test-first for pure logic modules and protocol parsing.
- Keep hardware-facing code behind small interfaces so simulation stays possible.
- New features need a short plan in `docs/exec-plans/active/`.
- Record important decisions in `docs/design-docs/`.
- Do not copy open-source code into the repo without a clear reason and attribution.

## Git Rules
- Main branch is protected by habit: no direct feature work on `main`.
- Branch prefixes:
  - `feat/<topic>`
  - `fix/<topic>`
  - `docs/<topic>`
  - `chore/<topic>`
  - `test/<topic>`
- Commit style:
  - `feat: ...`
  - `fix: ...`
  - `docs: ...`
  - `chore: ...`
  - `test: ...`

## Session Protocol
1. Read `AGENTS.md`, `WORKFLOW.md`, and the active plan before coding.
2. Check `git status` before making changes.
3. Make the smallest useful change that moves the current task forward.
4. Run the relevant validation before ending the session.
5. Update the active plan and related docs after meaningful progress.

## Knowledge Base
- Project workflow: `WORKFLOW.md`
- Architecture rules: `ARCHITECTURE.md`
- Active plans: `docs/exec-plans/active/`
- Completed plans: `docs/exec-plans/completed/`
- Design notes: `docs/design-docs/`
- Product notes: `docs/product-specs/`
