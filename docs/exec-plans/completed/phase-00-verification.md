# Phase 0 - Automated verification completion

## Goal
Finish the existing Windows PowerShell + GCC skeleton without feature code.

## Design and steps
- Add tests/smoke.ps1 as the sole combined build/test entry point.
- Check exit codes, output streams, and compiler warnings using GCC -Werror.
- Check executable creation in build.ps1; preserve normal build behavior.
- Verify failing expectations, compiler warnings and build errors in an ignored
  isolated copy under build/, without changing production main.c.
- Preserve the old PCAP plan as reference and write a Phase 1 environment plan.
- Update instructions, record actual results and commit only this task's files.

## Validation
Clean build, standalone build, smoke test and direct --help in child PowerShell
processes with exit-code checks. Verify Git root, origin and ignored artifacts.

## Non-goals
No capture, parsers, headers, dependencies, push or changes to the parent repository.

## Completion
Completed: strict build and all four CLI cases passed; isolated wrong-expectation,
warning and invalid-C probes returned nonzero. Production source is unchanged.
Clean/build/smoke/direct-help verification was performed in child PowerShell
processes. No dependency was added. See TESTING.md for evidence and limitations.
