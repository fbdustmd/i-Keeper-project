# NetSentry Project Specification

## 1. Problem

Plain HTTP can expose application data directly in network payloads.

NetSentry demonstrates this by reconstructing simple TCP streams and checking HTTP requests for fields that look sensitive.

## 2. Primary User

The primary user is the project owner, who is learning:
- C
- libpcap
- TCP/IP
- packet parsing
- network security analysis

This is primarily a learning and portfolio project.

## 3. MVP Inputs

Primary:
- Live capture from a selected interface (Phase 1, per 2026-09-28 user instruction).
- `.pcap` files produced from controlled or authorized lab traffic for reproducible analysis.

Later:
- HTTPS/TLS metadata analysis only; encrypted payload analysis remains excluded.

## 4. MVP Outputs

CLI output containing:
- flow endpoints
- HTTP method/path when available
- detected sensitive field names
- masked values
- simple warnings

## 5. MVP Requirements

### Packet layer
- Read PCAP with libpcap.
- Parse Ethernet II.
- Parse IPv4.
- Parse TCP.
- Print/retain:
  - MAC addresses
  - IP addresses
  - ports
  - sequence number
  - ACK number
  - flags
  - payload length

### Flow layer
- Group packets by TCP connection.
- Treat reverse-direction packets as the same connection.
- Track each direction separately.

### Reassembly layer
- Store TCP payload segments.
- Order by sequence number.
- Build contiguous data for simple streams.
- Do not require full TCP-stack correctness.

### HTTP layer
- Recognize basic HTTP/1.x requests.
- Parse request line.
- Parse headers.
- Identify body boundary.
- Support simple `Content-Length`.
- Support form-urlencoded bodies first.

### Detection layer
Detect field/header names such as:
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
- Cookie
- Authorization

Values must be masked in normal output.

## 6. Non-Goals

The MVP is not intended to:
- decrypt HTTPS
- steal credentials
- act as a proxy
- modify packets
- block traffic
- implement a production IDS
- implement a full TCP stack
- support every HTTP encoding
- process Internet-scale traffic
- provide a GUI

## 7. Security and Ethics

Use only:
- self-generated captures
- lab traffic
- traffic the user is authorized to analyze

Do not design features whose main purpose is collecting usable credentials.

Default output should reveal field presence, not secrets.

## 8. MVP Acceptance Test

Given a controlled PCAP containing a request logically equivalent to:

```http
POST /login HTTP/1.1
Host: test.local
Content-Type: application/x-www-form-urlencoded
Cookie: session=abcdef

username=test&password=1234
```

NetSentry should be able to produce output logically equivalent to:

```text
TCP Flow:
client:ephemeral → server:80

HTTP:
POST /login

Sensitive fields:
- Cookie
- username
- password

Values:
********
```

If that complete path works using modular code, the MVP is considered complete.
