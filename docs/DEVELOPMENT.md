# NetSentry 개발 안내

NetSentry는 C 기반 패킷 분석과 TCP 재조립을 배우는 프로젝트다. 이 문서는 개발 구조, 작은 작업 요청 방법과 사용자 검토 시점을 안내한다. 개발 규칙은 루트 AGENTS.md를 따른다.

## 권장 저장소 구조

```text
NetSentry/
├── AGENTS.md
├── ARCHITECTURE.md
├── README.md
├── build.ps1
├── src/
├── include/
├── tests/
├── samples/
├── docs/
│   ├── DEVELOPMENT.md
│   ├── PROJECT_SPEC.md
│   ├── IMPLEMENTATION_RULES.md
│   ├── TESTING.md
│   ├── ROADMAP.md
│   ├── EXECUTION_PLANS.md
│   └── exec-plans/
│       ├── active/
│       ├── completed/
│       └── reference/
└── Makefile
```

## AI에 작업을 요청하는 방법

매번 프로젝트 전체 설명을 붙여 넣을 필요는 없다. 다음처럼 범위를 좁혀 요청한다.

```text
docs/ROADMAP.md에 따라 Phase 2 실행 계획을 작성하라.
AGENTS.md를 따르고 Phase 3 이후는 구현하지 마라.
변경 결과를 빌드하고 테스트하라.
```

또는:

```text
docs/exec-plans/reference/phases-02-to-05-packet-parsers.md를 참고해
Ethernet 파서만 구현하라.
docs/IMPLEMENTATION_RULES.md의 패킷 경계 검사 규칙을 따르라.
구현 후 관련 테스트를 실행하고 남은 제한사항을 보고하라.
```

## 사용자 검토 시점

1. 모듈·API 설계 후
2. 각 파서 구현 후
3. 연결 관리 자료구조 설계 후
4. TCP 재조립 구현 전
5. 재조립 알고리즘 변경 후
6. MVP 완료 선언 전

목표는 동작하는 코드와 프로젝트 이해를 함께 얻는 것이다.
프로젝트 소유자가 코드와 네트워크 개념을 설명할 수 있어야 한다.
