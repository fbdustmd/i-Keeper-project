# Phase 1 - Packet Capture Environment

Status: Next

## Goal
Prepare and verify the capture dependency for the existing Windows PowerShell +
MSYS2 UCRT64 GCC toolchain, then design the smallest live-capture module.

## Current State
The CLI skeleton builds. tests/smoke.ps1 verifies existing behavior.
No capture library, SDK, driver, interface access or capture code is verified.

## Non-goals
No parsers, stream processing, TLS decryption or change to the primary toolchain.

## Design
First inventory installed capture runtime/SDK and architecture compatibility with
GCC. Determine header paths, link libraries, runtime DLLs, driver and permissions.
Record evidence before choosing setup steps. Do not assume GCC alone provides
libpcap. Future capture.c owns opening, iteration and closing; main.c handles CLI.
Public interfaces are designed before adding capture.h.

## Steps
- [ ] Inspect installed runtime/SDK, compiler target and available interfaces.
- [ ] Document the supported dependency setup and exact build/link commands.
- [ ] Verify dependency headers/link/runtime in a minimal environment check.
- [ ] Explain interface selection, caplen vs wire length and capture termination.
- [ ] Write the live-capture implementation/test plan before feature code.

## Validation
Keep the Phase 0 smoke test passing. Record dependency versions and evidence of
successful header/link/runtime checks. Later live capture must print packet length,
handle invalid interfaces and terminate safely on authorized lab traffic.

## Risks
SDK/compiler ABI mismatch, missing driver, permissions and interface naming.

## Completion Notes
Not started. Capture setup is Phase 1 work, not an unverified Phase 0 claim.
