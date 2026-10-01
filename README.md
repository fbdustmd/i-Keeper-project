# NetSentry

처음 실행한다면 [명령어 사용설명서](docs/USER_GUIDE.md)의 준비·빌드·실습 순서를 따라가세요.

## 프로젝트 소개

NetSentry는 평문 통신에서 민감정보가 노출되는 과정을 이해하기 위한 **C 기반 네트워크 분석 학습 프로젝트**입니다. 패킷 분석부터 TCP 연결 관리와 스트림 재조립까지 단계적으로 직접 구현합니다.

**현재 Phase 1:** 사용자가 선택한 인터페이스에서 패킷을 수집하고 번호·캡처 길이·원래 길이·링크 타입·캡처 시각(PC 현지 시간, 마이크로초)을 출력합니다. 원본 패킷 내용은 출력하거나 저장하지 않습니다.

**미구현:** Ethernet·IPv4·TCP 파서, PCAP 파일 입력, 연결 관리, 스트림 재조립, HTTP 분석, 민감 필드 탐지. HTTPS 복호화는 목표가 아닙니다.

## 개발 환경과 빌드

Windows + PowerShell 7 + MSYS2 UCRT64 x64 GCC + Npcap 런타임·SDK를 사용합니다.
SDK는 Git에 포함하지 않습니다. 최초 준비는 [개발 환경](docs/ENVIRONMENT.md)을 따릅니다.

프로젝트 루트에서 실행합니다.

```powershell
.\build.ps1 -WarningsAsErrors
.\build\netsentry.exe --help
.\build\netsentry.exe --list
```

C11, `-Wall -Wextra -Wpedantic -Werror`로 검증합니다. 한글 경로에서 GCC 임시 파일 오류를 피하기 위해 `-save-temps=obj`를 사용합니다.
빌드 시 설치된 Npcap DLL 두 개를 Git 제외 `build/`에 복사합니다. 전역 PATH나 드라이버 설정은 변경하지 않습니다.
`.\build.ps1 -Clean`은 프로그램 빌드 산출물과 복사한 DLL을 정리합니다.
Makefile은 동일한 PowerShell 명령을 호출하는 Windows용 대체 진입점이며, 현재 환경에서는 make가 없어 미검증입니다.

## 실행 방법

`--list`에 나온 이름을 `--interface`에 직접 지정합니다. 다음은 로컬 loopback에서 10개 또는 5초 중 먼저 도달한 조건으로 종료하는 예입니다.

```powershell
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --count 10 --duration 5
```

- `--count`: 1~1000000, 기본 100개
- `--duration`: 1~86400초, 기본 30초. 장치 설정이 끝난 뒤 캡처 루프의 시간입니다.
- `--filter`: 선택적 BPF 캡처 필터. 예: `--filter 'ip and tcp port 80'`
- Ctrl+C: 트래픽이 없어도 종료 요청을 처리하고 캡처 핸들을 닫습니다.
- 정상 종료는 0, 인자·장치·캡처 오류는 1입니다.

기본 캡처는 비 promiscuous 모드이며 인터페이스를 자동 선택하지 않습니다.
이 PC의 loopback은 Ethernet이 아닌 `DLT_NULL`입니다. 현재는 링크 타입을 기록할 뿐 헤더를 분석하지 않습니다.

## 테스트 방법

```powershell
# 실제 캡처를 열지 않는 검증
.\tests\smoke.ps1
.\tests\unit.ps1

# 허가된 로컬 실습: 인터페이스를 명시해야 실행 가능
.\tests\capture.ps1 -Interface '\Device\NPF_Loopback'
```

2026-09-30 검증: 기본 CLI 17개, 모의 캡처 13개 시나리오와 잘못된 설정·없는 장치, 실제 loopback 패킷 5개(각 64바이트), 무트래픽 시간 제한·Ctrl+C, 잘못된 BPF 필터 처리 통과. 컴파일 경고 없음.
자세한 근거와 제한은 [테스트 안내](docs/TESTING.md)를 참고합니다.

## 프로젝트 구조

```text
NetSentry/
├── AGENTS.md
├── ARCHITECTURE.md
├── README.md
├── build.ps1
├── Makefile
├── src/             # main.c: CLI, capture.c: 캡처 수명 관리
├── include/         # capture.h
├── tests/           # 기본·모의·실시간 검증
├── samples/         # 향후 실습용 PCAP, 현재 안내 문서
└── docs/            # 명세·설계·검증·실행 계획
```

## 최종 목표와 개발 로드맵

패킷 캡처(Packet Capture) → Ethernet → IPv4 → TCP → 흐름 식별(Flow Tracking)
→ TCP 스트림 재조립(TCP Stream Reassembly) → HTTP/1.x 요청 → 민감 필드 탐지·마스킹.

Phase 0·1은 검증했고 다음은 Phase 2 Ethernet 파서입니다.
PCAP 입력은 Phase 5, 연결 관리는 Phase 6, 제한적 재조립은 Phase 7에 구현합니다.
전체 단계는 [로드맵](docs/ROADMAP.md), 모듈 경계는 [아키텍처](ARCHITECTURE.md)에 있습니다.

## Git 작업 방식

이 폴더는 독립 Git 저장소입니다. 상위 작업 공간의 별도 저장소와 원본 ZIP은 보존하며 합치거나 서브모듈로 추가하지 않습니다.

```powershell
Set-Location -LiteralPath 'C:\Users\류연승\Documents\ChatGPT\키퍼 프로젝트\NetSentry'
git rev-parse --show-toplevel
git status
```

원격은 https://github.com/fbdustmd/i-Keeper-project.git 입니다.
2026-09-30 확인한 원격 기본 브랜치와 현재 작업 브랜치는 `codex/project-bootstrap`입니다.
이번 구조 정리와 Phase 1은 별도 로컬 커밋으로 관리하며 push·merge·기본 브랜치 변경은 하지 않습니다.
이후 게시할 때도 실제 구현·검증 상태를 확인하고 한국어 설명과 커밋 메시지를 사용합니다.

[개발 안내](docs/DEVELOPMENT.md) · [구조 정리 기록](docs/REPOSITORY_CLEANUP.md) · [이전 캡처 코드 검토](docs/LEGACY_CAPTURE_REVIEW.md)

## 한글 출력

PowerShell 스크립트는 UTF-8 BOM을 유지한다. 실행 파일은 콘솔에 Unicode를 직접 출력하므로
직접 실행할 때 chcp나 시스템 로캘을 바꿀 필요가 없다. 파일·파이프 출력은 UTF-8이다.
콘솔 회귀 테스트는 `.\tests\console.ps1`로 실행한다.
`--count 5`만 지정하면 인터페이스 누락 오류가 정상이다. 캡처에는 `--interface`가 필요하다.
