# Phase 2~5 참고 계획 - Ethernet / IPv4 / TCP 분석

> Phase 2는 Ethernet, Phase 3은 IPv4, Phase 4는 TCP, Phase 5는 통합이다.
> 각 단계를 따로 구현·검증한다. 이 문서가 전체 동시 구현을 허용하는 것은 아니다.

## 목표

하나의 캡처 패킷을 Ethernet → IPv4 → TCP 순으로 분석하고
검증된 메타데이터를 다음 모듈에 제공한다.

## 제외 범위

연결 관리, TCP 재조립, HTTP 파서, 민감 필드 탐지.

## 필수 결과

정상 IPv4/TCP 패킷에서 출발지·목적지 MAC, IP, 포트,
SEQ, ACK, 플래그, TCP 페이로드(Payload) 포인터와 길이를 얻는다.

## 설계 제약

- 각 계층에서 caplen을 검사한다.
- IPv4 헤더 길이는 IHL, TCP 헤더 길이는 Data Offset으로 계산한다.
- ntohs / ntohl을 사용한다.
- 페이로드는 C 문자열이 아니라 바이트 범위로 취급한다.
- 모듈 경계가 불분명해진다면 파서 내부에서 다음 파서를 호출하지 않는다.

## 작업 순서

- [ ] 파서 상태·결과 타입 정의
- [ ] Ethernet 파서 구현
- [ ] Ethernet 파서 테스트
- [ ] IPv4 파서 구현
- [ ] IHL과 잘린 입력 테스트
- [ ] TCP 파서 구현
- [ ] Data Offset과 잘린 입력 테스트
- [ ] TCP 페이로드 범위의 안전한 계산
- [ ] Wireshark와 값 비교
- [ ] 잘못되거나 잘린 패킷 테스트를 하나 이상 추가

## 검증

결과를 아는 PCAP의 MAC, IP, 포트, 원시 SEQ/ACK, 페이로드 길이를 Wireshark와 비교한다.

## 위험 요소

캡처 범위를 넘겨 읽기, IP/TCP 헤더를 고정 20바이트로 가정하기,
원시 시퀀스 번호를 Wireshark의 상대 번호와 비교하기.
