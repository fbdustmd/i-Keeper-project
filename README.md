# NetSentry

C와 libpcap으로 패킷을 분석하는 학습용 CLI 프로젝트입니다.
최종 흐름은 캡처 → Ethernet → IPv4 → TCP → Flow → 제한적 재조립
→ HTTP/1.x → 민감 필드 탐지입니다. 민감 값은 마스킹하며 HTTPS를 복호화하지 않습니다.

## 현재 지원 기능

Phase 0 최소 CLI: 기본 안내, `--help`, 잘못된 인자의 오류 종료.
패킷 캡처와 분석은 아직 구현되지 않았습니다. 현재 빌드에는 libpcap이 필요하지 않습니다.

## Windows 빌드 및 실행

PowerShell과 PATH에 등록된 GCC가 필요합니다. 프로젝트 루트에서 실행합니다.

```powershell
.\build.ps1
.\build\netsentry.exe --help
.\build.ps1 -Clean
```

빌드 스크립트는 자신의 위치를 기준으로 경로를 계산하므로 공백·한글 경로를 지원합니다.
`-std=c11 -Wall -Wextra -Wpedantic`으로 컴파일합니다.
Windows GCC의 한글 임시 경로 문제를 피하도록 `-save-temps=obj`로 중간 파일을
`build/`에 저장합니다. `-Clean`은 이 빌드가 생성한 실행 파일과 중간 파일을 제거합니다.

## make 환경

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
- `tests/`: 향후 단위·통합 테스트
- `samples/`: 향후 작은 실습용 PCAP
- `docs/`: 요구사항, 로드맵, 검증, 실행 계획
- `ARCHITECTURE.md`: 모듈 경계와 데이터 흐름
- `AGENTS.md`: 개발 규칙

개발 순서는 [ROADMAP](docs/ROADMAP.md), 검증 방법은 [TESTING](docs/TESTING.md)을 따릅니다.
Phase 1은 인터페이스 선택과 실시간 캡처입니다. 캡처 라이브러리·드라이버 및
실행 환경은 Phase 1 설계에서 확정하고 실제 패킷으로 검증합니다.

## Git

이 디렉터리를 독립 저장소 루트로 사용합니다.
상위 `키퍼 프로젝트`의 기존 저장소와 ZIP은 보존하며 이 저장소에 포함하지 않습니다.
Git 명령은 이 디렉터리에서 실행하고, 상위 저장소에서 이 폴더를 서브모듈로 추가하지 않습니다.
origin은 https://github.com/fbdustmd/i-Keeper-project.git 입니다.
각 Phase는 기능 브랜치에서 검증 후 커밋합니다.
