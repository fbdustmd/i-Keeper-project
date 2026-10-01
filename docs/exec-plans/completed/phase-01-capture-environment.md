# Phase 1 - 패킷 캡처 환경

상태: 완료 — 2026-09-30 (현재 PC의 loopback 범위 검증)

## 목표

Windows PowerShell + MSYS2 UCRT64 GCC 환경에서 캡처 의존성을 준비·검증하고
최소 실시간 캡처 모듈을 설계한다.

## 착수 당시 상태 (과거 기록)

CLI 기본 구조는 빌드 가능하며 tests/smoke.ps1이 기존 동작을 검증한다.
Npcap 서비스와 DLL 존재는 확인했지만 SDK, 링크, 인터페이스 접근, 실제 캡처는 미검증이다.

## 제외 범위

파서, 스트림 처리, TLS 복호화, 주 개발 도구 변경은 포함하지 않는다.

## 설계

설치된 캡처 런타임·SDK와 GCC 아키텍처 호환성을 먼저 조사한다.
헤더 경로, 링크 라이브러리, 런타임 DLL, 드라이버, 권한을 확인한 뒤 설정을 정한다.
GCC만으로 libpcap이 제공된다고 가정하지 않는다.
capture.c는 열기·순회·닫기를 맡고 main.c는 CLI를 맡는다.
capture.h는 공개 인터페이스를 설계한 뒤 추가한다.

## 작업 순서

- [x] 런타임·SDK, 컴파일 대상, 사용 가능한 인터페이스 확인
- [x] 의존성 구성과 정확한 빌드·링크 명령 기록
- [x] 최소 환경 확인으로 헤더·링크·런타임 검증
- [x] 인터페이스 선택, caplen과 원래 길이 차이, 캡처 종료 설명
- [x] 기능 코드 작성 전에 실시간 캡처 구현·테스트 계획 확정

## 검증

Phase 0 테스트를 유지하고 의존성 버전과 헤더·링크·런타임 검증 근거를 기록한다.
이후 실제 캡처는 허가된 실습 트래픽에서 길이 출력, 잘못된 인터페이스 처리,
안전한 종료를 검증해야 한다.

## 위험 요소

SDK·컴파일러 ABI 불일치, 드라이버 누락, 권한, 인터페이스 이름 차이.

## 조사 기록 — 2026-09-28

- GCC 대상: x86_64-w64-mingw32 (UCRT64)
- npcap 서비스: 실행 중
- System32/Npcap/wpcap.dll 및 Packet.dll: 존재
- Program Files/Npcap: 존재
- UCRT64 기본 include/pcap.h: 없음

다른 위치의 SDK 부재를 뜻하지는 않는다. SDK 위치·버전은 미확인이고 캡처 코드는 아직 없다.

## 환경 검증 후 구현 계획

1. Npcap SDK의 Include 헤더(pcap.h/pcap 헤더)와 x64 import library를 찾는다.
   현재 GCC에서 wpcap.lib 링크가 되는지 확인하고 Packet.lib는 선택 API·검증에서 필요할 때 사용한다.
   MSVC 예제만으로 GCC 호환성을 판단하지 않는다. 의존성 바이너리는 Git에 넣지 않는다.
2. 컴파일·링크 확인 후 build.ps1에 SDK 헤더·라이브러리 설정을 추가한다.
   Npcap 런타임 폴더의 wpcap.dll과 의존 DLL 로딩을 확인한다. 전역 PATH를 영구 변경하지 않는다.
3. Npcap AdminOnly 설정과 사용할 계정의 인터페이스 접근을 확인한다.
   권한 오류는 보고하고 캡처 권한을 조용히 높이지 않는다.
4. include/capture.h와 src/capture.c에 인터페이스 나열·선택, pcap_open_live,
   pcap_next_ex 결과 처리, pcap_close 책임을 설계한다.
   src/main.c는 인자 처리와 실행 연결만 맡긴다.
5. 횟수·시간 제한과 Ctrl+C 종료를 설계한다. 트래픽이 없는 인터페이스에서도 종료가 무한 대기하지 않아야 한다.
6. 잘못된 인터페이스·오류 처리와 명시적으로 선택하는 실습용 실시간 캡처 테스트를 추가한다.
   도움말 테스트를 유지하고 패킷 번호, caplen, 원래 길이만 출력한다. 페이로드는 출력하지 않는다.
7. 통제된 테스트와 패킷 수·길이를 비교하고 링크 타입을 기록한다.
   Npcap loopback의 DLT_NULL을 이후 Ethernet II로 해석하면 안 된다.

예상 파일: src/main.c, src/capture.c, include/capture.h, build.ps1,
tests/capture.ps1, README.md, docs/TESTING.md.
위 파일 목록은 착수 전 계획이었다. 실제 구현과 검증 결과는 아래 완료 기록을 따른다.

참고: [Npcap 개발자 안내](https://npcap.com/guide/npcap-devguide.html),
[Npcap API 문서](https://npcap.com/guide/wpcap/pcap.html).

## Phase 1 실제 구현 계획 — 2026-09-30

- 현재 런타임은 Npcap 1.87 / libpcap 1.10.6, GCC 대상은 x86_64-w64-mingw32다.
- SDK 1.16을 사용자 허용 후 공식 ZIP에서 .deps/npcap-sdk로 압축 해제했다.
- 최소 C 프로그램으로 헤더·x64 wpcap.lib 링크·런타임 호출·장치 열거를 검증했다.
- main.c: --list, --interface, --count, --duration, --filter 인자 검증만 담당한다.
- capture.h/c: 장치 열거, 명시적 장치 열기, 링크 타입, 비차단 순회, 시간·수·Ctrl+C 종료를 담당한다.
- 기본 수 제한 100개, 시간 제한 30초. 먼저 도달한 조건에서 종료한다.
- 선택적 BPF 필터는 허가된 실습 트래픽을 다른 loopback 통신과 분리하기 위해 제공한다.
- 데이터는 pcap_next_ex가 성공한 동안만 참조한다. 본문 복사·출력·프로토콜 파싱은 하지 않는다.
- Npcap 시스템 DLL을 로컬 build/로 복사해 명시적인 실행 의존성을 제공한다. 전역 PATH 변경이나 DLL 재배포는 하지 않는다.
- 일반 smoke test는 캡처를 열지 않는다. 실시간 tests/capture.ps1은 반드시 인터페이스 인자를 받아야 한다.
- 사용자 승인 대상은 Npcap loopback의 로컬 실습 트래픽이다. 다른 장치는 캡처하지 않는다.
- 실패 경로의 자원 정리는 가짜 pcap 함수로 별도 검사하고 실제 loopback의 수 제한·무트래픽 시간 제한·Ctrl+C를 확인한다.
- Phase 2 진입 전에 결과를 기록한다. TCP 연결 관리(Phase 6)와 재조립(Phase 7)은 유지한다.
- PCAP 입력은 Phase 5 통합 단계에서 추가하며, 캡처 루프가 제공하는 바이트·길이·링크 타입을 같은 파서에 넘긴다.

## 완료 기록 — 2026-09-30

- 독립 NetSentry 폴더로 이동한 후 Phase 0의 네 기본 사례를 재검증했다. 구조 정리 커밋은 `0b056ec`이다.
- SDK 준비 전 공식 출처·기존 런타임·아키텍처를 다시 확인했다. 사용자의 최종 동의에 따라 SDK를 Git 제외 폴더에 준비하고 loopback으로만 실습했다.
- main.c / capture.c / capture.h를 분리하고 목록·명시적 선택·길이·링크 타입·BPF·횟수·시간·Ctrl+C 종료를 구현했다.
- 엄격한 빌드, CLI 17개, 모의 캡처 13개 시나리오와 잘못된 설정·장치 검증을 통과했다.
- 실시간 검증: 5개 × 64바이트, DLT_NULL, 무트래픽 시간 제한 2.10초, Ctrl+C 종료 1.82초, 잘못된 필터 처리 통과.
- Ctrl+C 무시 속성이 부모에서 상속되는 경우를 재현해 제품에서 해제하도록 보완했다.
- SDK·DLL·빌드 결과·실습 원본 데이터는 커밋하지 않는다. 실제 Wi-Fi/Ethernet·관리자 전용 권한 거부·장시간 부하·메모리 진단 도구는 미검증이다.
- 다음 작업은 Phase 2 Ethernet II 파서다. 이번에는 파서나 연결·재조립 코드를 구현하지 않았다.

현재 검증의 상세 조건·제한은 [TESTING](../../TESTING.md), 환경은 [ENVIRONMENT](../../ENVIRONMENT.md)에 있다.
이 계획은 완료 기록으로 이동했으며 reference/의 과거 초안과 구분한다.
