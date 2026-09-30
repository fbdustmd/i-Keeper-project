# 저장소 정리 기록 — 2026-09-30

## 위치와 보존

프로젝트를 작업 공간 아래 `NetSentry/`로 이동했다.
이전 위치는 `netsentry_harness_instructions/netsentry_harness/`다.
`.git`, 기존 커밋, origin, Git 제외 빌드 파일을 함께 보존했다.
상위 작업 공간의 별도 `.git`과 원본 ZIP은 변경하지 않았다.

이동 전 내부 저장소에는 사용자 변경이 없었다. 추가 중첩 저장소와
실행 중인 NetSentry·GCC·디버거·make 프로세스도 발견되지 않았다.
스크립트는 `$PSScriptRoot`를 사용해 이동 후에도 동작했고,
README의 절대 경로를 수정했다. 과거 경로는 이 기록에만 남긴다.

## 개발 안내

`HARNESS_README.md`를 `docs/DEVELOPMENT.md`로 이동했다.
개발 규칙은 루트 `AGENTS.md`, 현재 계획은 `docs/exec-plans/active/`,
과거 기록은 `completed/`와 `reference/`에서 관리한다.

## GitHub 확인

- 원격: https://github.com/fbdustmd/i-Keeper-project.git
- 기본 브랜치: `codex/project-bootstrap`
- 정리 시작 커밋: `4c7e4c4f6e1cddc53865ea5b33eb75e05d47453d`
- 원격 README: Phase 0 완료·Phase 1 준비 상태
- 저장소 소개: 비어 있음

기본 브랜치와 이력을 확인했으므로 기존 브랜치에서 작업을 이어간다.
main 생성·기본 브랜치 변경·push·merge는 이번 작업에 포함하지 않는다.
구조 정리와 Phase 1 구현은 별도 로컬 커밋으로 기록한다.

GitHub 소개에 사용할 문구:
> C와 Npcap 기반 네트워크 분석 학습 프로젝트. 패킷 캡처에서 TCP 연결 관리·스트림 재조립과 평문 민감정보 탐지까지 단계적으로 구현합니다.

## 이동 후 검증

`tests/smoke.ps1`의 엄격 빌드와 기존 네 CLI 사례가 모두 통과했다.
컴파일 경고가 없고 Git 루트·기존 HEAD·origin이 유지됨을 확인했다.
