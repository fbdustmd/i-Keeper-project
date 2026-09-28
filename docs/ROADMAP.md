# NetSentry 개발 로드맵

사용자의 2026-09-28 지시를 기준으로 합니다. 검증한 단계만 완료 표시합니다.

Phase 0 - 프로젝트 기반 구성: 완료
Phase 1 - 패킷 캡처 환경: 다음 작업

현재 진입점: `docs/exec-plans/active/phase-01-capture-environment.md`.

- [x] Phase 0: 최소 프로젝트·빌드·Git 구성 (Windows GCC 빌드·CLI 검증 완료)
- [ ] Phase 1: 인터페이스 선택, pcap_open_live, 캡처 루프, 안전한 종료
- [ ] Phase 2: Ethernet 파서
- [ ] Phase 3: IPv4 파서
- [ ] Phase 4: TCP 파서
- [ ] Phase 5: 패킷 파서 통합 및 실제 패킷 대조
- [ ] Phase 6: 양방향 Flow 식별과 방향별 상태
- [ ] Phase 7: 제한적인 TCP 스트림 재조립
- [ ] Phase 8: HTTP/1.x 요청 분석
- [ ] Phase 9: 민감 필드 규칙 탐지와 마스킹
- [ ] Phase 10: CLI 결과 통합과 전체 수용 테스트

각 Phase는 계획, 구현, 빌드·테스트, 결과 검토, 커밋 순으로 진행합니다.
Flow, 재조립, HTTP는 구현 전에 개념과 설계를 검토합니다.
PCAP 입력은 재현 가능한 검증을 위해 유지할 요구사항이며 실시간 캡처를 대체하지 않습니다.

기존 PCAP 우선 계획은 `exec-plans/reference/legacy-pcap-bootstrap.md`에 보존했습니다.
파서 초안은 `exec-plans/reference/phases-02-to-05-packet-parsers.md`에 보존했습니다.
각 단계 착수 시 해당 계획을 구체화하며 한 번에 전체를 구현하지 않습니다.
