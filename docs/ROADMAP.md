# NetSentry 개발 로드맵

사용자의 2026-09-28 단계 구분을 유지한다. 2026-09-30 현재 Phase 0·1을 검증했다.

- [x] Phase 0: 독립 프로젝트·빌드·Git 기반
- [x] Phase 1: 인터페이스 목록·명시적 선택, 실시간 캡처, 링크 타입·길이 출력, 수·시간·Ctrl+C 종료
- [ ] Phase 2: Ethernet II 파서, 최소 길이·EtherType 검증
- [ ] Phase 3: IPv4 파서, IHL·전체 길이·지원 범위 검증
- [ ] Phase 4: TCP 파서, Data Offset·포트·SEQ/ACK 검증
- [ ] Phase 5: 파서 통합, PCAP 파일 입력, 고정 실습 PCAP와 실제 패킷 대조
- [ ] Phase 6: 양방향 Flow 식별과 방향별 상태
- [ ] Phase 7: 제한적 TCP 스트림 재조립, 연속 데이터·기본 순서 역전·중복·단순 재전송
- [ ] Phase 8: HTTP/1.x 요청 분석
- [ ] Phase 9: 민감 필드 규칙 탐지와 마스킹
- [ ] Phase 10: CLI 결과 통합과 전체 수용 테스트

다음 작업은 **Phase 2 Ethernet II 파서의 고정 바이트 배열 테스트와 최소 구현**이다.
실시간 loopback 검증에서 확인한 `DLT_NULL`을 Ethernet으로 해석하면 안 된다.
Ethernet 파서는 `DLT_EN10MB`에서만 연결하며 다른 링크 타입은 명시적으로 미지원 처리한다.
Phase 2 자체 검증은 합성 Ethernet 프레임으로 할 수 있어 추가 인터페이스 캡처 권한이 필요하지 않다.

PCAP 입력은 Phase 5에서 실시간 입력과 같은 바이트·caplen·원래 길이·링크 타입을 파서에 전달하도록 연결한다.
TCP 연결 관리(Phase 6)와 재조립(Phase 7)은 최종 목표의 필수 항목이다.
완전한 TCP 스택·모든 중첩 정책·HTTPS 복호화로 범위를 넓히지 않는다.

Phase 1 결과: [완료 실행 계획](exec-plans/completed/phase-01-capture-environment.md).
파서 참고 초안: [Phase 2~5](exec-plans/reference/phases-02-to-05-packet-parsers.md).
과거 PCAP 우선 계획은 [참고 자료](exec-plans/reference/legacy-pcap-bootstrap.md)에 보존했다.
현재 진행 중인 실행 계획은 없다. 다음 Phase 착수 시 참고 초안을 구체화해 active/에 작성한다.
