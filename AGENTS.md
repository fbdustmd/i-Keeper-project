# NetSentry Agent Instructions

## 1. Project Mission

NetSentry is a learning-oriented network security project written in C with libpcap.

The 2026-09-28 user roadmap takes precedence: Phase 0 establishes the build;
Phase 1 implements live capture using a selected interface and pcap_open_live.
PCAP input remains a reproducible analysis/testing requirement.
See docs/ROADMAP.md for the current Phase 0-10 numbering.

The analysis pipeline is:

PCAP
→ Ethernet parsing
→ IPv4 parsing
→ TCP parsing
→ TCP Flow tracking
→ limited TCP Stream Reassembly
→ HTTP/1.x Request parsing
→ plaintext sensitive-field detection
→ CLI report

The goal is NOT to build Wireshark, an IDS/IPS, a full TCP stack, or an HTTPS decryption tool.

## 2. First Rule: Keep the Scope Small

Before implementing a feature, verify that it belongs to the current MVP.

MVP includes:
- PCAP input
- Ethernet / IPv4 / TCP parsing
- TCP Flow classification
- basic TCP reassembly
- HTTP Request parsing
- sensitive field detection
- CLI output

MVP excludes:
- GUI
- machine learning
- HTTPS payload decryption
- IPv6
- UDP
- full TCP state-machine implementation
- complete overlapping-segment handling
- SACK
- high-performance traffic processing
- production IDS/IPS behavior

Do not add excluded features unless the user explicitly asks for an extension after MVP completion.

## 3. Source of Truth

Use these documents depending on the task:

- `ARCHITECTURE.md`
  - module boundaries
  - data flow
  - ownership of responsibilities

- `docs/PROJECT_SPEC.md`
  - product scope
  - MVP requirements
  - non-goals
  - completion criteria

- `docs/IMPLEMENTATION_RULES.md`
  - C coding rules
  - parser rules
  - memory and boundary handling

- `docs/TESTING.md`
  - validation strategy
  - required tests
  - Wireshark comparison rules

- `docs/EXECUTION_PLANS.md`
  - how to create and update implementation plans

- `docs/exec-plans/active/`
  - current implementation plan

Do not read every document for every trivial edit.
Read the document relevant to the current task.

## 4. Required Working Style

For non-trivial work:

1. Understand the task.
2. Inspect only the relevant existing files.
3. Identify the smallest implementation unit.
4. Write or update an execution plan when the task spans multiple modules.
5. Implement one coherent step.
6. Build.
7. Run relevant tests.
8. Compare packet-level results with expected values or Wireshark where applicable.
9. Summarize:
   - what changed
   - why
   - tests performed
   - remaining limitations

Do not silently make broad architectural changes.

## 5. Learning-Oriented Constraint

This project belongs to a beginner learning networking and C.

Prefer:
- explicit code
- readable structures
- small functions
- clear ownership
- simple data structures first

Avoid:
- unnecessary abstractions
- clever macros
- premature optimization
- hidden control flow
- complex generic frameworks
- code that is difficult for the project owner to explain

When there are two correct designs, prefer the easier one to explain unless it materially harms correctness.

## 6. Network Parsing Safety Rules

Never assume packet data is long enough.

Before reading a header or payload:
- validate `caplen`
- validate computed offsets
- validate IPv4 IHL
- validate TCP Data Offset
- ensure the next header fits inside captured data

Never trust packet contents.

Use network byte-order conversions where required:
- `ntohs`
- `ntohl`

Do not cast and dereference packet memory without checking required length first.

## 7. TCP Reassembly Scope

For MVP, implement only enough TCP reassembly to reconstruct simple HTTP requests.

Required:
- identify flow
- separate directions
- record sequence number
- record payload
- order segments by sequence
- concatenate contiguous payload

Preferred after baseline works:
- exact duplicate suppression
- simple retransmission handling
- basic out-of-order handling

Do NOT turn reassembly into a complete TCP stack.

If a capture contains an unsupported edge case:
- detect it if practical
- fail safely or skip the ambiguous data
- document the limitation

## 8. HTTP Scope

MVP HTTP parsing targets HTTP/1.x requests.

Initial support:
- Request Line
- Headers
- Body
- `Content-Length`
- `application/x-www-form-urlencoded`

Sensitive indicators include:
- password
- passwd
- pwd
- username
- userid
- email
- token
- access_token
- refresh_token
- session
- sessionid
- session_id
- Cookie header
- Authorization header

Do not store or print real sensitive values in normal output.
Mask values with `********`.

## 9. HTTPS Scope

Do not attempt to break or bypass TLS.

For HTTPS/TLS traffic, analysis is limited to metadata such as:
- source/destination IP
- source/destination port
- protocol
- packet length
- flow information
- TLS presence when detectable

The encrypted application payload is outside the MVP.

## 10. Module Boundaries

Keep responsibilities separated.

- `capture.c`
  - libpcap input only

- `ethernet.c`
  - Ethernet parsing only

- `ipv4.c`
  - IPv4 parsing only

- `tcp.c`
  - TCP header/payload metadata parsing only

- `flow.c`
  - TCP flow identity and direction management

- `reassembly.c`
  - TCP segment ordering and stream construction

- `http.c`
  - HTTP parsing

- `detector.c`
  - sensitive field detection

`main.c` coordinates modules.
It must not become a second implementation of every module.

## 11. Completion Rule

A task is not complete just because code was written.

A task is complete only when:
- the project builds
- relevant tests pass
- packet bounds are checked
- no unrelated behavior was changed
- limitations are documented
- the result can be explained by the project owner

If a test cannot be run, explicitly state why.
