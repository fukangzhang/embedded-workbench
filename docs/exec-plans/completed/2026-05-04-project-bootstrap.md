# Goal
Bootstrap the repository so future embedded project work follows a stable engineering workflow from day one.

## Scope
- Initialize Git locally
- Add project-level guide documents
- Define branch, commit, PR, and plan conventions
- Add a minimal environment check script

## Steps
1. Create repository guidance files
2. Add planning and documentation folders
3. Add GitHub contribution template
4. Add init script
5. Review the scaffold and explain the next GitHub step

## Validation
- `git status --short --branch`
- Read `AGENTS.md`, `ARCHITECTURE.md`, and `WORKFLOW.md`
- Run `powershell -ExecutionPolicy Bypass -File scripts/init.ps1`

## Open Questions
- What GitHub repository name should be used
- Whether the first tracked target is simulation-first or board-first
- Whether CI should be added now or after the first testable module exists

## Progress Log
- 2026-05-04: Initialized local Git repository on `main`.
- 2026-05-04: Added guide documents, PR template, plan folders, and init script.
