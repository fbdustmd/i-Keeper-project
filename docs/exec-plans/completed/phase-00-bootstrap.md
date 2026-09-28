# Phase 0 - Minimal build and repository

## Goal
Create a reproducible C CLI skeleton in this directory and an independent Git repository.

## Non-goals
No packet capture, parsers, dependency installation, or remote push.

## Current State
Nine instruction/design documents exist. No sources, tests or build system exist.
The parent directory has an unborn Git repository; preserve it and its ZIP.
GCC is available on Windows; make is not on PATH.

## Design
Use src/main.c for a small help-only CLI. Provide build.ps1 for the current
Windows environment and a Makefile for environments with make and a POSIX shell.
Use C11 and -Wall -Wextra -Wpedantic. Keep future module directories documented.
Initialize this directory on codex/project-bootstrap and configure origin.
Align roadmap references with the user's Phase 0-10 sequence.

## Steps
- [x] Add skeleton, build commands and documentation.
- [x] Build and check no-argument, help and invalid-argument behavior.
- [x] Rebuild from clean output and inspect Git exclusions.
- [x] Review the verified Phase 0 files and prepare the bootstrap commit.

## Validation
Run build.ps1 and exercise the generated executable with exit-code checks.
Inspect git diff --check and staged files before committing.
Makefile execution is unverified until make is available.

## Risks
The nested repository must not be accidentally staged as a submodule in the parent.
Phase 1 still requires a capture backend and runtime environment decision.

## Completion Notes
Phase 0 development environment completion (2026-09-28): automated smoke tests
now build with -Werror, check four CLI exit codes and stdout/stderr, and return
nonzero on failure. Negative probes in an ignored copy demonstrated assertion,
warning and compilation failure handling. README and TESTING provide one command.
The repository root and origin are verified; build/ is ignored. Windows
PowerShell 7.6.5 + MSYS2 UCRT64 GCC 16.1.0 is the verified primary environment.
Historical PCAP/parser plans are preserved in reference/; the active next plan
is phase-01-capture-environment.md. Phase 1 can begin with dependency inspection;
capture runtime/SDK readiness is not claimed. No feature code was changed.

Windows GCC build passed without warnings; four CLI cases passed with expected
exit codes. Clean removed all generated files and rebuild succeeded.
See docs/TESTING.md for the reproduced temporary-path failure and verified fix.
The Makefile remains unexecuted. Capture and parsing are not implemented.
