# NetSentry Architecture

## 1. System Goal

NetSentry transforms packet data into a small, understandable security finding.

```text
PCAP
  ↓
Capture
  ↓
Ethernet Parser
  ↓
IPv4 Parser
  ↓
TCP Parser
  ↓
Flow Manager
  ↓
TCP Reassembly
  ↓
HTTP Parser
  ↓
Sensitive Data Detector
  ↓
CLI Report
```

## 2. Design Principle

Each module answers one question.

- Capture: What bytes were captured?
- Ethernet: Which L2 frame is this?
- IPv4: Which hosts and upper protocol are involved?
- TCP: Which ports, sequence numbers, flags, and payload exist?
- Flow: Which logical TCP conversation owns this packet?
- Reassembly: What ordered byte stream can be reconstructed?
- HTTP: Is the byte stream an HTTP request and what are its fields?
- Detector: Does the HTTP message contain plaintext sensitive indicators?

## 3. Data Model

Recommended conceptual structures:

```c
typedef struct {
    const uint8_t *data;
    size_t caplen;
    size_t wirelen;
} PacketView;
```

```c
typedef struct {
    uint8_t src_mac[6];
    uint8_t dst_mac[6];
    uint16_t ethertype;
    size_t payload_offset;
} EthernetInfo;
```

```c
typedef struct {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint8_t protocol;
    uint16_t total_length;
    size_t header_length;
    size_t payload_offset;
} IPv4Info;
```

```c
typedef struct {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq;
    uint32_t ack;
    uint8_t flags;
    size_t header_length;
    const uint8_t *payload;
    size_t payload_length;
} TCPInfo;
```

Exact field names may change, but ownership must remain clear.

## 4. Flow Identity

A TCP connection is identified logically by:

- endpoint A IP
- endpoint A port
- endpoint B IP
- endpoint B port
- protocol

Because packets travel in both directions, a single connection should contain two independent stream directions.

```text
Flow
├── A → B stream
└── B → A stream
```

Sequence numbers are tracked independently per direction.

## 5. Reassembly Boundary

MVP reassembly is intentionally limited.

It should reconstruct simple contiguous streams using sequence numbers.

It does not need to fully implement:
- TCP receive windows
- congestion control
- SACK
- all overlap policies
- all retransmission patterns
- sequence-wrap edge cases

Unsupported ambiguity should not be hidden.

## 6. HTTP Boundary

The HTTP layer receives reconstructed TCP bytes.

The HTTP parser must not:
- capture packets
- perform flow lookup
- sort TCP segments
- detect security findings directly

The detector receives parsed HTTP fields and decides whether a sensitive field exists.

## 7. Error Handling

Parsers should prefer explicit status results.

Example:

```c
typedef enum {
    PARSE_OK = 0,
    PARSE_TRUNCATED,
    PARSE_UNSUPPORTED,
    PARSE_INVALID
} ParseResult;
```

Malformed or truncated input must not cause out-of-bounds reads.

## 8. Evolution Order

Architecture should evolve in this order:

1. reliable parsing
2. correct flow identity
3. simple ordered reassembly
4. basic out-of-order handling
5. duplicate/retransmission handling
6. HTTP parsing
7. detection
8. reporting improvements

Do not optimize before correctness is demonstrated.
