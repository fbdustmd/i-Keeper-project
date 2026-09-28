# Phase 2 - Ethernet / IPv4 / TCP Parsers

> Scope mapping: this draft covers the current roadmap's Phase 2 (Ethernet),
> Phase 3 (IPv4), Phase 4 (TCP), and Phase 5 (integration). Implement and verify
> each separately; this file does not authorize a combined implementation.

## Goal

Parse one captured packet through:

```text
Ethernet → IPv4 → TCP
```

and expose validated metadata to later modules.

## Non-goals

Do not implement:
- Flow Manager
- TCP reassembly
- HTTP parser
- detector

## Required Results

For valid IPv4/TCP packets, obtain:
- source/destination MAC
- source/destination IP
- source/destination port
- sequence number
- ACK number
- flags
- TCP payload pointer
- TCP payload length

## Design Constraints

- Every layer checks `caplen`.
- IPv4 header length comes from IHL.
- TCP header length comes from Data Offset.
- Use `ntohs` / `ntohl`.
- Packet payload is a byte range, not assumed to be a C string.
- A parser must not call the next parser internally if that makes module boundaries unclear.

## Steps

- [ ] Define parser result/status type.
- [ ] Implement Ethernet parser.
- [ ] Test Ethernet parser.
- [ ] Implement IPv4 parser.
- [ ] Test IPv4 IHL and truncation handling.
- [ ] Implement TCP parser.
- [ ] Test TCP Data Offset and truncation handling.
- [ ] Compute TCP payload safely.
- [ ] Compare values with Wireshark.
- [ ] Add at least one malformed/truncated packet test.

## Validation

For a known PCAP, verify:
- MAC values
- IP values
- ports
- raw SEQ/ACK
- payload length

against Wireshark.

## Risks

- reading beyond captured packet
- assuming fixed 20-byte IP/TCP headers
- comparing raw sequence number with Wireshark relative sequence display
