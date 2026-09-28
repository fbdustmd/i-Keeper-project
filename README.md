# NetSentry

C와 libpcap으로 패킷을 분석하는 학습용 CLI 프로젝트입니다.
최종 흐름은 캡처 → Ethernet → IPv4 → TCP → Flow → 제한적 재조립
→ HTTP/1.x → 민감 필드 탐지입니다. 민감 값은 마스킹하며 HTTPS를 복호화하지 않습니다.

## Current Status

Phase 0 개발환경 검증 완료 / Phase 1 캡처 환경 준비 단계.

## Current Features

Phase 0 최소 CLI: 기본 안내, `--help`, 잘못된 인자의 오류 종료.
패킷 캡처와 분석은 아직 구현되지 않았습니다. 현재 빌드에는 libpcap이 필요하지 않습니다.

## Requirements

- Windows
- PowerShell (검증 버전은 docs/TESTING.md 참고)
- PATH에 등록된 MSYS2 UCRT64 GCC

## Build

PowerShell과 PATH에 등록된 GCC가 필요합니다. 프로젝트 루트에서 실행합니다.

```powershell
.\build.ps1
```

빌드 스크립트는 자신의 위치를 기준으로 경로를 계산하므로 공백·한글 경로를 지원합니다.
`-std=c11 -Wall -Wextra -Wpedantic`으로 컴파일합니다.
Windows GCC의 한글 임시 경로 문제를 피하도록 `-save-temps=obj`로 중간 파일을
`build/`에 저장합니다. `-Clean`은 이 빌드가 생성한 실행 파일과 중간 파일을 제거합니다.
정리가 필요할 때만 `.\build.ps1 -Clean`을 실행한 뒤 다시 빌드합니다.

## Run

```powershell
.\build\netsentry.exe --help
```

## Test

```powershell
.\tests\smoke.ps1
```

한 명령으로 경고를 오류로 취급하는 빌드와 CLI 4개 사례를 검증합니다.
성공 시 `All smoke tests passed.`와 종료 코드 0, 실패 시 종료 코드 1입니다.

## make 환경

Primary Windows build path: build.ps1
Makefile is an optional Unix/MSYS2 build path.

GCC, GNU make와 POSIX 셸이 있는 환경을 위한 대체 경로입니다.
현재 Windows 환경에서는 make가 없어 이 경로는 아직 검증하지 않았습니다.

```sh
make
./build/netsentry --help
make clean
```

## 구조

- `src/`: 구현, 현재 main.c만 존재
- `include/`: 향후 모듈 헤더
- `tests/`: smoke.ps1과 테스트 안내; 기능별 테스트는 향후 추가
- `samples/`: 향후 작은 실습용 PCAP
- `docs/`: 요구사항, 로드맵, 검증, 실행 계획
- `ARCHITECTURE.md`: 모듈 경계와 데이터 흐름
- `AGENTS.md`: 개발 규칙

## Roadmap

개발 순서는 [ROADMAP](docs/ROADMAP.md), 검증 방법은 [TESTING](docs/TESTING.md)을 따릅니다.
Phase 1은 인터페이스 선택과 실시간 캡처입니다. 캡처 라이브러리·드라이버 및
실행 환경은 Phase 1 설계에서 확정하고 실제 패킷으로 검증합니다.

## Git

이 디렉터리를 독립 저장소 루트로 사용합니다.
상위 `키퍼 프로젝트`의 기존 저장소와 ZIP은 보존하며 이 저장소에 포함하지 않습니다.
Git 명령은 이 디렉터리에서 실행하고, 상위 저장소에서 이 폴더를 서브모듈로 추가하지 않습니다.
origin은 https://github.com/fbdustmd/i-Keeper-project.git 입니다.
각 Phase는 기능 브랜치에서 검증 후 커밋합니다.
검증과 diff 검토 후 해당 브랜치를 GitHub에 push합니다. main은 검증된 기준으로
유지하고, 이후 기능은 가능하면 PR로 검토하며 자동 merge나 force push는 하지 않습니다.
최초 게시 브랜치는 `codex/project-bootstrap`입니다. main 기준 브랜치를 정하기
전에는 Phase 1 브랜치를 임의로 만들지 않습니다.

현재 작업 공간에서 명령 실행 전에 루트를 확인합니다.

```powershell
Set-Location -LiteralPath 'C:\Users\류연승\Documents\ChatGPT\키퍼 프로젝트\netsentry_harness_instructions\netsentry_harness'
git rev-parse --show-toplevel
git status
```

출력 루트는 위 netsentry_harness 경로여야 합니다. 상위 저장소는 수정하지 않습니다.
