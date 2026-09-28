# NetSentry Testing Guide

## Phase 0 verification (2026-09-28)

Environment: Windows PowerShell, MSYS2 UCRT64 GCC 16.1.0.
From the project root:

```powershell
.\build.ps1
.\build\netsentry.exe
.\build\netsentry.exe --help
.\build\netsentry.exe --invalid
.\build\netsentry.exe --help extra
.\build.ps1 -Clean
.\build.ps1
```

Verified: compilation with C11 / -Wall / -Wextra / -Wpedantic produced no warnings.
No arguments and --help printed usage and exited 0. Invalid and extra arguments
printed an error and exited 1. Clean removed the executable and intermediate files;
rebuild succeeded. Each native program exit code was checked immediately.

The initial GCC invocation failed because its assembler misread the Korean Windows
TEMP path. Setting relative TMPDIR did not resolve it. The Windows build now uses
-save-temps=obj and relative source/output paths to keep intermediate files in build/.
No global environment settings were changed.

Makefile execution is not yet verified because make is unavailable on PATH.
There are no packet, parser, integration or sanitizer results yet.
The remaining sections specify requirements for future phases.

## 1. Testing Philosophy

Do not test NetSentry only with one large real capture.

Use small captures where the expected result is known.

Tests should answer one question at a time.

## 2. Test Layers

### Parser unit tests

Test:
- Ethernet header
- IPv4 header
- TCP header
- invalid/truncated input

### Flow tests

Provide synthetic packet metadata and verify:
- same 5-tuple maps to same flow
- reversed endpoints map to same connection
- unrelated endpoints map to different flows
- directions are classified correctly

### Reassembly tests

Start with synthetic segments.

#### In order

```text
SEQ 1000 -> ABC
SEQ 1003 -> DEF
SEQ 1006 -> GHI
```

Expected:

```text
ABCDEFGHI
```

#### Out of order

```text
SEQ 1000 -> ABC
SEQ 1006 -> GHI
SEQ 1003 -> DEF
```

Expected:

```text
ABCDEFGHI
```

#### Exact duplicate

```text
SEQ 1000 -> ABC
SEQ 1003 -> DEF
SEQ 1003 -> DEF
SEQ 1006 -> GHI
```

Expected:

```text
ABCDEFGHI
```

### HTTP tests

Test:
- simple GET
- simple POST
- multiple headers
- form-urlencoded body
- missing body
- malformed request

### Detector tests

Input fields:
- password
- pwd
- email
- token
- Cookie
- Authorization

Expected:
- field name detected
- sensitive value masked

## 3. PCAP Integration Tests

Maintain small captures in `samples/`.

Recommended:
- `01_single_tcp_packet.pcap`
- `02_simple_http_get.pcap`
- `03_http_post_form.pcap`
- `04_http_split_segments.pcap`
- `05_out_of_order.pcap`
- `06_duplicate_segment.pcap`

Do not depend only on one giant capture.

## 4. Wireshark Validation

For packet parser validation, compare with Wireshark:
- source/destination MAC
- source/destination IP
- ports
- raw sequence number when needed
- ACK
- flags
- TCP header length
- payload length

Important:
Wireshark may show relative TCP sequence numbers by default.

When debugging sequence values, confirm whether you are comparing:
- relative sequence number
- raw sequence number

## 5. Build Validation

At minimum, development should use strong warnings.

Recommended baseline:

```bash
gcc -Wall -Wextra -Wpedantic
```

Add stricter warnings gradually when the existing code is clean.

## 6. Memory Validation

When available, use:
- AddressSanitizer
- UndefinedBehaviorSanitizer

Recommended debug build concept:

```bash
-fsanitize=address,undefined -fno-omit-frame-pointer
```

Fix parser and memory errors before adding new features.

## 7. Definition of Done

A feature is done only when:
- code builds
- relevant unit/integration test passes
- invalid input does not cause an out-of-bounds access
- output matches expected packet facts
- limitation is documented if behavior is incomplete

## 8. Regression Rule

When a bug is found:
1. create the smallest reproducible case
2. add a test that fails
3. fix the bug
4. verify the new test passes
5. verify existing tests still pass

Do not fix recurring parser bugs only by adding local special cases.
