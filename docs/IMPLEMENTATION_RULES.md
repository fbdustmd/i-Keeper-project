# NetSentry Implementation Rules

## 1. Language

Use C.

Prefer standard, readable C over macro-heavy or metaprogramming-like designs.

## 2. Function Design

Prefer small functions with one responsibility.

Good:
- `parse_ethernet(...)`
- `parse_ipv4(...)`
- `parse_tcp(...)`
- `flow_find_or_create(...)`
- `reassembly_add_segment(...)`
- `http_parse_request(...)`
- `detect_sensitive_fields(...)`

Avoid large functions that parse multiple protocol layers and perform detection at once.

## 3. Packet Bounds

Every parser must receive enough information to validate bounds.

Do not rely on:
- expected packet sizes
- minimum header assumptions
- null termination inside packet payloads

Before reading N bytes from offset O:

```text
O + N <= caplen
```

must be true.

## 4. IPv4 Rules

Read IHL from the packet.

```text
IPv4 header length = IHL × 4
```

Reject or skip invalid values.

Do not assume 20 bytes unless verified.

## 5. TCP Rules

Read TCP Data Offset.

```text
TCP header length = data_offset × 4
```

Do not assume 20 bytes unless verified.

Convert multibyte fields:
- ports with `ntohs`
- seq/ack with `ntohl`

## 6. Payload Length

Payload length should be computed from validated packet/IP/TCP lengths.

Never read beyond `caplen`.

If captured bytes are shorter than wire-reported bytes, treat the packet as truncated.

## 7. Flow Key

Flow comparison must treat both directions as one connection.

Do not incorrectly create two independent flows for:

```text
A:50000 → B:80
B:80 → A:50000
```

However, stream state inside that flow must remain direction-specific.

## 8. Reassembly

Start simple.

Iteration order:
1. in-order segments
2. out-of-order storage and sorting
3. exact duplicates
4. simple retransmissions

Do not start with interval trees, advanced overlap resolution, or sequence-space algorithms unless later required.

## 9. Memory

Every allocation must have a clear owner.

For every allocated structure, document:
- who allocates it
- who frees it

Avoid copying entire packets when a small parsed structure is enough.

For reassembly payloads that must outlive libpcap's packet buffer, copy only the required payload bytes.

## 10. Strings

Packet payloads are byte arrays, not guaranteed C strings.

Never call string functions on packet data unless:
- the data was copied into a buffer
- enough space exists
- a null terminator was explicitly added

## 11. Sensitive Values

Detector output must prefer:

```text
field=password value=********
```

over printing real values.

## 12. Logging

Logs should help learning and debugging.

Useful:
- packet number
- flow id
- direction
- seq
- payload length
- parser result

Avoid dumping raw sensitive payload by default.

## 13. Refactoring

Refactor only after:
- current behavior is covered by tests
- the new boundary is clearer than the old one
- the refactor is relevant to the current task

Do not rewrite unrelated modules.
