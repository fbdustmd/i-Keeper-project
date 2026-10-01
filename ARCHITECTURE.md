# NetSentry 아키텍처

## 1. 시스템 목표

패킷 데이터를 이해하기 쉬운 보안 분석 결과로 바꾼다.

```text
PCAP
  ↓
패킷 입력
  ↓
Ethernet 분석
  ↓
IPv4 분석
  ↓
TCP 분석
  ↓
연결 관리
  ↓
TCP 스트림 재조립
  ↓
HTTP 분석
  ↓
민감 필드 탐지
  ↓
CLI 보고
```

## 2. 설계 원칙

각 모듈은 하나의 질문에 답한다.

- 캡처: 어떤 바이트를 수집했는가?
- Ethernet: 어떤 L2 프레임인가?
- IPv4: 어떤 호스트와 상위 프로토콜이 관련되는가?
- TCP: 포트, 시퀀스 번호, 플래그, 페이로드(Payload)는 무엇인가?
- 연결 관리: 어떤 TCP 연결에 속하는가?
- 재조립: 어떤 순서의 바이트 스트림을 복원할 수 있는가?
- HTTP: HTTP 요청인가? 어떤 필드가 있는가?
- 탐지: 평문 민감 필드가 있는가?

## 3. 데이터 모델

다음은 권장 개념 구조다. 실제 필드 이름은 바꿀 수 있지만 소유권은 명확해야 한다.

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

## 4. 연결 식별

TCP 연결은 끝점 A의 IP·포트, 끝점 B의 IP·포트, 프로토콜로 식별한다.
패킷은 양방향으로 이동하므로 한 연결 안에 독립적인 두 스트림을 둔다.

```text
연결
├── A → B 스트림
└── B → A 스트림
```

시퀀스 번호(Sequence Number)는 방향별로 추적한다.

## 5. 재조립 경계

MVP는 시퀀스 번호를 이용해 단순한 연속 스트림을 복원한다.
TCP 수신 윈도, 혼잡 제어, SACK, 모든 중첩 정책과 재전송 패턴,
시퀀스 번호 순환 경계까지 완전히 구현할 필요는 없다.
지원하지 않는 모호한 상황을 숨기지 않는다.

## 6. HTTP 경계

HTTP 계층은 재조립된 TCP 바이트를 받는다.
패킷 캡처, 연결 조회, 세그먼트 정렬, 보안 결과 판정을 직접 맡지 않는다.
탐지 모듈은 분석된 HTTP 필드를 받아 민감 필드의 존재 여부를 판단한다.

## 7. 오류 처리

파서는 다음과 같이 명시적인 상태를 반환하는 방식을 우선한다.

```c
typedef enum {
    PARSE_OK = 0,
    PARSE_TRUNCATED,
    PARSE_UNSUPPORTED,
    PARSE_INVALID
} ParseResult;
```

잘못되거나 잘린 입력이 범위 밖 메모리 읽기를 일으키면 안 된다.

## 8. 확장 순서

1. 신뢰할 수 있는 패킷 분석
2. 정확한 연결 식별
3. 순서대로 도착한 데이터 재조립
4. 기본 순서 역전 처리
5. 중복·재전송 처리
6. HTTP 분석
7. 민감 필드 탐지
8. 출력 개선

정확성을 입증하기 전에 최적화하지 않는다.

## 9. Phase 1의 실제 경계 — 2026-09-30

현재 구현은 `main.c`의 인자 검증과 `capture.c`의 Npcap 수명 관리까지다.
위의 PacketView·파서·연결·재조립 구조는 후속 설계이며 아직 C 코드에 구현하지 않았다.

```text
main.c: CaptureOptions 생성
  → capture_run: 명시적 장치 확인 → 열기 → 링크 타입·BPF·비차단 모드 설정
    → pcap_next_ex: 캡처 헤더 + Npcap 소유 바이트
      → 현재: 번호·caplen·원래 길이만 출력
      → 종료 조건: 수 제한 / 시간 제한 / Ctrl+C / 오류
    → pcap_close
```

`caplen`은 실제 접근 가능한 바이트 수이고 `len`은 원래 패킷 길이다.
둘이 다르면 잘린 캡처일 수 있으므로 이후 파서는 `len`을 메모리 읽기 한도로 사용하면 안 된다.
현재는 caplen > len을 오류 처리하고, caplen < len은 그대로 메타데이터를 출력한다.
패킷 바이트는 복사하지 않는다. 향후 파서는 다음 읽기 전까지 빌린 바이트를 사용하고,
재조립처럼 오래 보관해야 할 데이터만 해당 모듈이 복사·해제한다.

Phase 5 PCAP 입력도 같은 분석 함수에 바이트·길이·링크 타입을 전달한다.
입력 루프가 파일/실시간 종료 조건을 관리하고 파서는 입력 출처를 알 필요가 없도록 한다.
지금은 미사용 콜백 프레임워크나 추상 입력 계층을 만들지 않는다.

시간 측정은 단조 증가하는 `GetTickCount64`를 사용한다.
비차단 읽기에서 패킷이 없으면 10ms 쉬고 종료 플래그와 시간을 다시 확인한다.
Ctrl+C 처리기는 원자적으로 플래그만 바꾸고 핸들은 주 실행 흐름이 한 번 닫는다.
Windows 콘솔의 상속된 Ctrl+C 무시 설정도 해제한다.

근거: [Npcap 비차단 API](https://npcap.com/guide/wpcap/pcap_setnonblock.html),
[Windows 콘솔 종료 처리](https://learn.microsoft.com/en-us/windows/console/setconsolectrlhandler).
