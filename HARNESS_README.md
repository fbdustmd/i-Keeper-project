# NetSentry AI 개발 작업 안내

이 폴더는 AI와 함께 개발할 때 사용할 저장소 지침을 담고 있다.

## 권장 저장소 구조

```text
netsentry/
├── AGENTS.md
├── ARCHITECTURE.md
├── HARNESS_README.md
├── README.md
├── build.ps1
├── src/
├── include/
├── tests/
├── samples/
├── docs/
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
docs/exec-plans/active/phase-01-capture-environment.md에 따라 Phase 1을 준비하라.
AGENTS.md를 따르고 Phase 2는 구현하지 마라.
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
