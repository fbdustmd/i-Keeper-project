# Phase 1 - Project Bootstrap and PCAP Input

> Historical draft: superseded as the immediate execution plan by the user's
> Phase 0-10 roadmap (docs/ROADMAP.md). Bootstrap is now Phase 0; Phase 1 requires
> live capture. Retain the PCAP details as reference; rewrite the capture plan
> before Phase 1 implementation. Commands below describe future behavior.

## Goal

Create a minimal NetSentry repository that:
- builds successfully
- opens a PCAP file
- iterates through packets
- prints packet index and captured length

## Non-goals

This phase does not implement:
- Ethernet parsing
- IPv4 parsing
- TCP parsing
- flow tracking
- reassembly
- HTTP parsing

## Current State

Assume the repository may be empty or contain earlier experimental libpcap code.

Reuse existing code only when it is clear and compatible with the architecture.

## Design

Initial source layout:

```text
src/
├── main.c
└── capture.c

include/
└── capture.h
```

Responsibilities:

- `main.c`
  - validate arguments
  - start capture processing
  - report fatal errors

- `capture.c`
  - open PCAP
  - iterate packets
  - close PCAP

## Steps

- [ ] Create basic directory structure.
- [ ] Create Makefile.
- [ ] Add `capture.h`.
- [ ] Implement PCAP open/close.
- [ ] Implement packet iteration with `pcap_next_ex`.
- [ ] Print packet number and `caplen`.
- [ ] Add argument validation.
- [ ] Build with warnings enabled.
- [ ] Run against one known PCAP.
- [ ] Record result below.

## Validation

Expected command shape:

```bash
make
./netsentry samples/example.pcap
```

Expected output shape:

```text
Packet #1 caplen=...
Packet #2 caplen=...
```

## Risks

- libpcap development package not installed
- Makefile library flags missing
- treating `pcap_next_ex` return values incorrectly

## Completion Notes

Fill in after implementation.
