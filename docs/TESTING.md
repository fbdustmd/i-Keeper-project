# NetSentry 테스트 안내

## 빌드 검증

주 개발환경은 Windows, PowerShell 7.6.5, MSYS2 UCRT64 GCC 16.1.0이다.
상위 저장소가 아닌 NetSentry 저장소 루트에서 실행한다.

```powershell
git rev-parse --show-toplevel
.\build.ps1 -Clean
.\build.ps1
```

빌드 스크립트는 GCC 존재를 확인하고 build/를 생성한다.
C11과 -Wall -Wextra -Wpedantic을 적용하며 컴파일 종료 코드와 실행 파일 생성을 검사한다.
`-WarningsAsErrors`는 -Werror를 추가한다. 기본 동작 검증에서는 항상 이 옵션을 사용한다.
Makefile은 Unix/MSYS2용 선택 경로이며 현재 환경에서 실행 검증하지 않았다.

## 기본 동작 검증(Smoke Test)

```powershell
.\tests\smoke.ps1
```

한 명령으로 빌드한 뒤 다음을 확인한다.

| 입력 | 예상 종료 코드 | 출력 |
|---|---:|---|
| 인자 없음 | 0 | stdout에 사용법, stderr는 비어 있음 |
| --help | 0 | stdout에 사용법, stderr는 비어 있음 |
| --invalid | 1 | stderr에 오류, stdout은 비어 있음 |
| --help extra | 1 | stderr에 오류, stdout은 비어 있음 |

각 CLI 사례의 실행 제한 시간은 10초다.

## 성공 기준

컴파일 경고가 없고 네 사례의 종료 코드와 출력 스트림이 모두 맞아야 한다.
성공 시 `모든 기본 동작 검증을 통과했습니다.`를 출력하고 0으로 종료한다.
프로그램 메시지와 테스트의 출력 읽기는 UTF-8을 기준으로 한다.

## 실패 시 확인

빌드, 경고, 프로그램 실행, 시간 초과, 기대값 검사 중 하나라도 실패하면 smoke.ps1은 1로 종료한다.
PATH의 GCC, 컴파일 진단, 실패한 사례 이름을 확인한다.
실행 직후 `$LASTEXITCODE`를 확인한다.
별도 프로세스의 종료 코드는 다음 명령으로 확인할 수 있다.

```powershell
pwsh -NoProfile -File .\tests\smoke.ps1
```

2026-09-28에 build/verification-probe의 Git 제외 복사본으로 실패 감지를 검증했다.
예상 종료 코드를 9로 바꾸자 테스트가 1로 종료했고,
사용하지 않는 static 변수를 추가하자 -Werror 빌드와 테스트가 실패했다.
잘못된 C 입력도 build.ps1의 비정상 종료로 이어졌다.
당시 실제 src/main.c는 변경하지 않았다. 이 복사본은 배포 테스트가 아닌 임시 검증 자료다.

## 한국어화 후 검증 — 2026-09-28

설명 문서와 CLI·빌드·테스트 메시지를 한국어로 바꾼 뒤 정리 후 빌드,
기본 동작 네 사례, 직접 --help 실행을 다시 확인했다. 컴파일 경고는 없었다.
별도 복사본에서 예상 종료 코드를 9로 바꾸면 한국어 오류와 종료 코드 1이 나왔다.
테스트의 UTF-8 출력 읽기를 명시했으며 옵션·종료 코드·식별자는 유지했다.

## 향후 테스트 계획

Phase 1에서는 캡처 관련 검증을 추가한다.
이후 파서 단계에서 고정 바이트 배열 단위 테스트와 PCAP 통합 사례를 추가한다.
현재 패킷 테스트나 캡처 검증은 없다.

## Phase 0 최초 검증 기록 — 2026-09-28

Windows PowerShell과 MSYS2 UCRT64 GCC 16.1.0에서 다음 순서로 확인했다.

```powershell
.\build.ps1
.\build\netsentry.exe
.\build\netsentry.exe --help
.\build\netsentry.exe --invalid
.\build\netsentry.exe --help extra
.\build.ps1 -Clean
.\build.ps1
```

C11 / -Wall / -Wextra / -Wpedantic 빌드에 경고가 없었다.
인자 없음·--help는 0, 잘못된 인자·추가 인자는 1로 종료했다.
정리 후 실행 파일과 중간 파일이 제거됐고 재빌드도 성공했다.
각 실행 직후 종료 코드를 확인했다. 당시 출력은 영어였으며 한국어화 후에도 종료 코드 계약은 유지한다.

처음에는 GCC assembler가 한글 Windows TEMP 경로를 잘못 읽어 실패했다.
상대 TMPDIR 설정만으로는 해결되지 않았다.
Windows 빌드는 -save-temps=obj와 상대 입력·출력 경로로 중간 파일을 build/에 둔다.
전역 환경 설정은 변경하지 않았다.

PATH에 make가 없어 Makefile은 미검증이다.
패킷·파서·통합 테스트와 sanitizer 실행 결과는 아직 없다.
아래 내용은 이후 단계의 검증 요구사항이다.

## 1. 테스트 원칙

큰 실제 캡처 하나에만 의존하지 않는다.
예상 결과가 분명한 작은 캡처를 사용하고 테스트마다 하나의 질문을 확인한다.

## 2. 계층별 테스트

### 파서 단위 테스트

Ethernet·IPv4·TCP 헤더와 잘못되거나 잘린 입력을 검사한다.

### 연결 테스트

합성 패킷 메타데이터로 같은 5-tuple은 같은 연결에 속하는지,
끝점이 뒤집혀도 같은 연결인지, 무관한 끝점은 분리되는지,
방향 구분이 정확한지 확인한다.

### 재조립 테스트

합성 세그먼트부터 시작한다.

순서대로 도착한 경우:

```text
SEQ 1000 -> ABC
SEQ 1003 -> DEF
SEQ 1006 -> GHI
```

순서가 뒤바뀐 경우:

```text
SEQ 1000 -> ABC
SEQ 1006 -> GHI
SEQ 1003 -> DEF
```

완전 중복이 있는 경우:

```text
SEQ 1000 -> ABC
SEQ 1003 -> DEF
SEQ 1003 -> DEF
SEQ 1006 -> GHI
```

세 경우 모두 예상 결과는 `ABCDEFGHI`다.

### HTTP 테스트

단순 GET·POST, 여러 헤더, form-urlencoded 본문, 본문 없음, 잘못된 요청을 확인한다.

### 탐지 테스트

입력 필드 `password`, `pwd`, `email`, `token`, `Cookie`, `Authorization`에 대해
필드 이름 탐지와 민감 값 마스킹을 확인한다.

## 3. PCAP 통합 테스트

samples/에 작은 캡처를 유지한다. 권장 파일은 다음과 같다.

- `01_single_tcp_packet.pcap`
- `02_simple_http_get.pcap`
- `03_http_post_form.pcap`
- `04_http_split_segments.pcap`
- `05_out_of_order.pcap`
- `06_duplicate_segment.pcap`

## 4. Wireshark 비교

출발지·목적지 MAC과 IP, 포트, 원시 SEQ, ACK, 플래그, TCP 헤더 길이,
페이로드(Payload) 길이를 비교한다.
Wireshark는 기본적으로 상대 시퀀스 번호를 표시할 수 있으므로
원시 값과 상대 값 중 무엇을 비교하는지 먼저 확인한다.

## 5. 컴파일 경고

최소 기준:

```bash
gcc -Wall -Wextra -Wpedantic
```

기존 코드의 경고를 해결한 뒤 더 엄격한 경고를 점진적으로 추가한다.

## 6. 메모리 검증

사용 가능하면 AddressSanitizer와 UndefinedBehaviorSanitizer를 사용한다.

```bash
-fsanitize=address,undefined -fno-omit-frame-pointer
```

새 기능보다 파서·메모리 오류 해결을 우선한다.

## 7. 완료 기준

빌드와 관련 단위·통합 테스트가 통과하고, 잘못된 입력에서 범위를 벗어나지 않아야 한다.
출력은 예상 패킷 정보와 일치해야 하며 미지원 동작은 제한사항으로 기록한다.

## 8. 회귀 방지

버그를 발견하면 최소 재현 사례 → 실패 테스트 → 수정 → 새 테스트 통과
→ 기존 테스트 통과 순으로 확인한다.
반복되는 파서 버그를 국소 예외 처리만 추가해 덮지 않는다.
