# Architecture

## Goal
This repository is an embedded project scaffold for a resume-grade MCU/RTOS project that can be developed before hardware is available.

## Layers
1. `hardware/`
   Hardware notes, pin mapping, board assumptions, and future schematics.
2. `bsp/`
   Board support package. Clock, pin init, startup integration, and MCU-specific glue.
3. `drivers/`
   Reusable drivers and protocol adapters for UART, I2C, SPI, sensors, storage, and actuators.
4. `app/`
   Application logic, FreeRTOS tasks, state machines, control policies, and feature orchestration.
5. `tests/`
   Host-side tests, protocol parsing tests, state-machine tests, and simulation stubs.
6. `docs/`
   Plans, design records, specs, and debugging notes.

## Dependency Rules
- `hardware` does not depend on code.
- `bsp` may depend on vendor SDK and startup files.
- `drivers` may depend on `bsp`, but not on `app`.
- `app` may depend on `drivers` and `bsp`, but should not know vendor details unless necessary.
- `tests` may depend on `app` and `drivers`, using fakes or stubs for hardware-facing code.

## Design Principles
- Prefer composition over global state.
- Keep parsing and business rules testable on the host machine.
- Isolate RTOS APIs behind task or service boundaries where practical.
- Keep module interfaces small and explicit.
- Add new folders only when they reduce real confusion.

## Initial Project Shape
The first real project should be able to run in two modes:
- simulation mode on the host machine for rapid iteration
- target mode for future STM32 board integration

## Open-Source Use Policy
- Open-source repositories may be used for reference, learning, and comparison.
- We do not present copied demos as original project work.
- If we adopt a third-party component, we document what was reused, why, and under which license.
