# Phase 1 - 패킷 캡처 환경

상태: 다음 작업

## 목표

Windows PowerShell + MSYS2 UCRT64 GCC 환경에서 캡처 의존성을 준비·검증하고
최소 실시간 캡처 모듈을 설계한다.

## 현재 상태

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

- [ ] 런타임·SDK, 컴파일 대상, 사용 가능한 인터페이스 확인
- [ ] 의존성 구성과 정확한 빌드·링크 명령 기록
- [ ] 최소 환경 확인으로 헤더·링크·런타임 검증
- [ ] 인터페이스 선택, caplen과 원래 길이 차이, 캡처 종료 설명
- [ ] 기능 코드 작성 전에 실시간 캡처 구현·테스트 계획 확정

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
추가 예정 파일은 아직 생성하지 않았다.

참고: [Npcap 개발자 안내](https://npcap.com/guide/npcap-devguide.html),
[Npcap API 문서](https://npcap.com/guide/wpcap/pcap.html).
