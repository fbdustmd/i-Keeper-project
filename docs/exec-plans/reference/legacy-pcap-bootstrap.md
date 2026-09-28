# 이전 초안 - 기본 구조와 PCAP 입력

> 현재 실행 계획이 아닌 과거 참고 자료다.
> 기본 구조는 현재 Phase 0으로 완료됐고 Phase 1은 실시간 캡처를 요구한다.
> PCAP 관련 내용은 참고용으로 보존한다. 단계 번호는 docs/ROADMAP.md를 따른다.
> 아래 명령과 체크리스트는 이 초안의 당시 상태이며 현재 구현 완료를 뜻하지 않는다.

## 목표

빌드 가능한 최소 저장소를 만들고 PCAP을 열어 패킷을 순회하며 번호와 캡처 길이를 출력한다.

## 제외 범위

Ethernet·IPv4·TCP 파서, 연결 추적, 재조립, HTTP 분석.

## 초안 작성 당시 가정

빈 저장소 또는 초기 libpcap 실험 코드가 있을 수 있다고 가정했다.
기존 코드는 명확하고 아키텍처에 맞는 경우에만 재사용한다.

## 설계

```text
src/
├── main.c
└── capture.c

include/
└── capture.h
```

main.c는 인자 검증, 캡처 처리 시작, 치명적 오류 보고를 맡는다.
capture.c는 PCAP 열기, 패킷 순회, 닫기를 맡는다.

## 당시 작업 목록

- [ ] 기본 디렉터리 구성
- [ ] Makefile 작성
- [ ] capture.h 추가
- [ ] PCAP 열기·닫기 구현
- [ ] pcap_next_ex로 패킷 순회
- [ ] 패킷 번호와 caplen 출력
- [ ] 인자 검증 추가
- [ ] 경고 옵션을 적용해 빌드
- [ ] 결과를 아는 PCAP으로 실행
- [ ] 결과 기록

## 검증 예시

```bash
make
./netsentry samples/example.pcap
```

이 초안의 실행 경로는 현재 build/netsentry.exe와 다르다.
예상 출력은 패킷 번호와 caplen의 나열이다.

## 위험 요소

libpcap 개발 패키지 누락, Makefile 링크 옵션 누락, pcap_next_ex 반환값의 잘못된 처리.

## 완료 기록

이 초안 자체는 완료 처리하지 않았다. 현재 단계의 결과는 completed/와 ROADMAP.md를 참고한다.
