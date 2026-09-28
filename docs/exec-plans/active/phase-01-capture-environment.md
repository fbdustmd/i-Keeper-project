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
Preparation inspected on 2026-09-28; no capture code implemented.

## Environment evidence
- GCC target: x86_64-w64-mingw32 (UCRT64).
- npcap service: Running.
- System32/Npcap/wpcap.dll and Packet.dll: present.
- Program Files/Npcap: present.
- UCRT64 default include/pcap.h: absent. This does not prove the SDK is absent
  elsewhere; SDK location and version remain unverified.

## Implementation plan after environment verification
1. Locate the Npcap SDK Include headers (pcap.h/pcap headers) and x64 import
   libraries. Verify wpcap.lib compatibility with this GCC; use Packet.lib only
   if required by the selected API/link test. Do not assume MSVC examples prove
   GCC linking works. No dependency binaries belong in Git.
2. Add explicit SDK include/library configuration to build.ps1 after a successful
   compile/link probe. Verify wpcap.dll and its dependencies load from Npcap's
   runtime directory; avoid permanent global PATH changes.
3. Check the installed Npcap AdminOnly setting and test interface access under
   the intended account. Report permission errors; do not silently elevate capture.
4. Design include/capture.h and src/capture.c for interface enumeration/selection,
   pcap_open_live, pcap_next_ex result handling and pcap_close ownership.
   src/main.c should only parse CLI arguments and coordinate execution.
5. Define bounded capture and Ctrl+C shutdown, including idle-interface behavior;
   verify the capture loop cannot hang indefinitely during shutdown.
6. Extend tests with invalid-interface/error handling and an explicit opt-in lab
   live-capture check. Preserve the existing help smoke tests. Print packet index,
   caplen and wire length only; do not print packet payloads.
7. Compare captured packet counts/lengths with a controlled test. Record link type:
   Npcap loopback uses DLT_NULL and must not later be treated as Ethernet II.

Expected files: src/main.c, src/capture.c, include/capture.h, build.ps1,
tests/capture.ps1, README.md and docs/TESTING.md. These are planned, not created.

References: [Npcap developer guide](https://npcap.com/guide/npcap-devguide.html)
and [Npcap API](https://npcap.com/guide/wpcap/pcap.html).
