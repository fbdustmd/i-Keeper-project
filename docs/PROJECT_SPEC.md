# NetSentry 프로젝트 명세

이 문서는 최종 MVP 목표다. 2026-09-30 현재 구현은 Phase 1 캡처까지이며, 아래 분석·재조립·탐지는 후속 요구사항이다.

## 1. 해결하려는 문제

평문 HTTP는 애플리케이션 데이터를 네트워크 페이로드(Payload)에 그대로 노출할 수 있다.
NetSentry는 단순한 TCP 스트림을 재조립하고 HTTP 요청에 민감 필드가 있는지 확인해 이를 보여준다.

## 2. 사용자와 목적

C, libpcap, TCP/IP, 패킷 분석, 네트워크 보안 분석을 배우는 프로젝트 소유자가 주 사용자다.
학습과 포트폴리오를 위한 프로젝트다.

## 3. MVP 입력

- 사용자의 2026-09-28 지시에 따라 Phase 1에서 선택한 인터페이스의 실시간 패킷 캡처(Packet Capture)
- 재현 가능한 분석을 위한 자체 생성·허가된 실습 트래픽의 `.pcap` 파일

HTTPS/TLS 메타데이터는 추후 후보이며 암호화된 페이로드 분석은 제외한다.

## 4. MVP 출력

CLI에 연결 양 끝점, 가능한 경우 HTTP 메서드·경로, 탐지한 민감 필드 이름,
마스킹된 값과 간단한 경고를 표시한다.

## 5. MVP 요구사항

### 패킷 계층

libpcap으로 PCAP을 읽고 Ethernet II, IPv4, TCP를 분석한다.
MAC·IP 주소, 포트, 시퀀스 번호(Sequence Number), ACK 번호, 플래그,
페이로드 길이를 출력하거나 보관한다.

### 연결 계층

패킷을 TCP 연결 단위로 묶는다. 역방향 패킷도 같은 연결로 분류하고 각 방향의 상태는 분리한다.

### 재조립 계층

TCP 페이로드 세그먼트를 저장하고 시퀀스 번호로 정렬해 단순한 연속 데이터를 만든다.
완전한 TCP 스택 수준의 정확성을 목표로 하지 않는다.

### HTTP 계층

기본 HTTP/1.x 요청을 인식하고 요청 줄, 헤더, 본문 경계를 분석한다.
단순한 `Content-Length`와 form-urlencoded 본문을 먼저 지원한다.

### 탐지 계층

다음 필드·헤더 이름을 탐지한다.
`password`, `passwd`, `pwd`, `username`, `userid`, `email`, `token`,
`access_token`, `refresh_token`, `session`, `sessionid`, `session_id`,
`Cookie`, `Authorization`.
일반 출력에서는 값을 가린다.

## 6. 제외 범위

HTTPS 복호화, 자격 증명 탈취, 프록시, 패킷 변조·차단, 운영용 IDS,
완전한 TCP 스택, 모든 HTTP 인코딩, 인터넷 규모 트래픽 처리, GUI는 구현하지 않는다.

## 7. 보안과 윤리

자체 생성 캡처, 실습 트래픽, 사용자가 분석 권한을 가진 트래픽만 사용한다.
사용 가능한 자격 증명 수집이 주목적인 기능을 설계하지 않는다.
기본 출력은 비밀 값이 아니라 필드의 존재를 보여준다.

## 8. MVP 수용 테스트

통제된 PCAP에 다음과 의미가 같은 요청이 있다고 가정한다.
아래는 개념 예제이며 실제 테스트 데이터는 지원하는 본문 길이 처리를 만족해야 한다.

```http
POST /login HTTP/1.1
Host: test.local
Content-Type: application/x-www-form-urlencoded
Cookie: session=abcdef

username=test&password=1234
```

결과는 TCP 연결의 클라이언트·서버 끝점, `POST /login`,
민감 필드 `Cookie`, `username`, `password`를 식별하고 값은 `********`로 가려야 한다.
모듈을 분리한 코드로 이 전체 흐름이 동작하면 MVP 완료로 본다.
