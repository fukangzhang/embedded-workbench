# Workflow

## Objective
Keep the project understandable for a beginner while still following an engineering workflow that will look credible in interviews.

## Default Loop
1. Define one small task.
2. Write or update a short plan in `docs/exec-plans/active/`.
3. Decide the validation for that task before editing code.
4. Implement the smallest viable change.
5. Run validation and capture the result.
6. Update docs, then commit.

## Planning Rules
- One active plan per meaningful task stream.
- Plans should include:
  - goal
  - scope
  - steps
  - validation
  - open questions
- Move finished plans to `docs/exec-plans/completed/`.

## TDD Guidance
Use test-driven development when the code is host-testable:
- protocol parsing
- command handling
- state machines
- thresholds and alarm rules
- data conversion and filtering

Use test-after for code that is mostly integration or startup wiring:
- build scripts
- vendor SDK integration
- board init
- RTOS task wiring

## GitHub Flow
1. Keep `main` stable.
2. Create a branch from `main`.
3. Make focused commits with a conventional prefix.
4. Open a PR with scope, validation, and risks.
5. Merge only after review and validation.

## Definition of Done
- Scope is limited and clear.
- Code or docs follow the architecture rules.
- Relevant tests or checks were run.
- The plan and project docs reflect the new state.
- The branch is ready for review or merge.
