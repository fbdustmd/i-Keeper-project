# NetSentry 테스트 안내

## 현재 검증 명령

Windows + PowerShell 7.6.5 + MSYS2 UCRT64 GCC 16.1.0에서 프로젝트 루트를 기준으로 실행한다.
Npcap 1.87, SDK 1.16 준비 방법은 [개발 환경](ENVIRONMENT.md)을 따른다.

```powershell
.\build.ps1 -Clean
.\tests\smoke.ps1
.\tests\unit.ps1
# 다음 명령만 실제 캡처를 연다. 실습 인터페이스를 명시해야 한다.
.\tests\capture.ps1 -Interface '\Device\NPF_Loopback'
```

## Phase 1 실제 결과 — 2026-09-30

| 검증 | 결과 | 근거 |
|---|---|---|
| 정리 후 재빌드 | 통과 | C11, -Wall -Wextra -Wpedantic -Werror, 경고 없음 |
| CLI 17개 | 통과 | 기본 4개 유지, 목록·누락·중복·숫자 범위·없는 장치 검사 |
| 모의 캡처 13개 시나리오 | 통과 | 초기화·열거·열기·링크·필터·비차단·읽기 실패와 정상·잘린 길이·무트래픽 |
| 잘못된 설정·없는 장치 | 통과 | 실패 반환, 없는 장치에서 open 호출 없음 |
| 모의 자원 수명 | 통과 | 모든 시나리오에서 open 수=close 수, 필터 컴파일 성공 시 freecode 호출 |
| 실제 loopback 패킷 수·길이 | 통과 | 로컬 UDP 5개, 각각 caplen=64, len=64, 수 제한 종료 |
| 링크 타입 | 통과 | 0 / DLT_NULL 확인; Ethernet으로 가정하지 않음 |
| 무트래픽 시간 제한 | 통과 | 2초 지정 후 약 2.1초에 패킷 0개로 종료 |
| 무트래픽 Ctrl+C | 통과 | 격리한 콘솔에서 CTRL_C_EVENT 전송 후 정상 종료·자원 정리 |
| 실제 잘못된 BPF | 통과 | 오류 종료 코드 1, 핸들 정리 메시지 확인 |
| 원본 출력 방지 | 통과 | 실습 데이터 문자열이 출력되지 않음 |

실습 패킷의 예상 길이는 DLT_NULL 4 + IPv4 20 + UDP 8 + 데이터 32 = 64바이트다.
UDP는 테스트 데이터를 만드는 수단이며 제품의 UDP 파서 기능은 아니다.
Ctrl+C 검증 중 상속된 무시 속성 때문에 종료되지 않는 상황을 발견했다.
제품이 이를 해제하도록 수정하고 테스트 부모가 무시 설정을 가진 조건에서도 다시 검증했다.

## 테스트 구분과 실패 처리

일반 테스트는 임의 인터페이스를 캡처하지 않는다. smoke의 --list는 열거만 한다.
모의 테스트는 실제 SDK 헤더로 컴파일하되 가짜 pcap 함수와 링크하므로 런타임을 열지 않는다.
실시간 테스트는 명시된 loopback과 임시 수신 포트에 한정하며 Wi-Fi·Ethernet을 열지 않는다.

테스트가 실패하면 비정상 종료 코드와 사례/예외가 출력된다.
smoke 사례는 최대 10초, 실시간 사례는 준비·종료 대기 제한을 둔다.
한글 출력은 UTF-8로 읽는다. 모의 실패 주입 중 stderr의 '오류' 메시지는 예상 동작이며 마지막 결과와 종료 코드를 확인한다.

## 남은 제한과 미검증

- 실제 Wi-Fi/Ethernet 캡처, 관리자 전용 Npcap 환경의 권한 거부는 미검증이다. 권한 오류는 모의 open 실패로만 검사했다.
- OS 핸들 누수 도구·sanitizer·장시간 부하 시험은 실행하지 않았다. 실제 종료와 모의 close/free 호출로 수명 관리를 확인했다.
- Windows PowerShell 5.1, 다른 컴파일러/운영체제, make 진입점은 미검증이다. 주 검증 환경은 PowerShell 7이다.
- Ctrl+C 이벤트는 테스트 콘솔에서 자동 전송했다. 모든 터미널 앱에서의 키보드 입력까지 검증한 것은 아니다.
- 시간 제한은 장치 준비 이후 캡처 루프에 적용된다. 드라이버 열기 지연·출력 스트림 정체·강제 프로세스 종료까지 보장하지 않는다.
- 고속 트래픽의 패킷 손실·통계는 범위 밖이다. 파서·PCAP 파일·연결·재조립 테스트는 해당 단계에서 추가한다.

아래는 과거 Phase 0 기록과 후속 단계 요구사항이다. 과거 기록을 현재 기능의 검증으로 대신하지 않는다.

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

## Windows PowerShell 한글 스크립트 수정 — 2026-10-01

Windows PowerShell 5.1은 BOM 없는 UTF-8 스크립트를 ANSI로 해석할 수 있다.
그 결과 한글 출력이 깨지고 smoke.ps1에서 문자열·해시 구문 오류가 발생했다.
build.ps1 및 tests/의 세 PowerShell 스크립트에 UTF-8 BOM을 추가했다.
Windows PowerShell에서 엄격 빌드와 기본 테스트 17개가 통과했고 PowerShell 7에서도 동일 테스트가 통과했다.
이 수정에서 실시간 캡처를 다시 실행하지는 않았다. 기존의 Windows PowerShell 5.1 미검증 표기는
이제 실시간·모의 테스트에 해당하며 빌드·smoke는 검증했다.

스크립트 실행 정책에 막히면 현재 창에서만 다음을 적용한 뒤 다시 실행한다.

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy RemoteSigned
.\tests\smoke.ps1
```

PowerShell 스크립트를 편집할 때 UTF-8 BOM을 유지한다.
참고: https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_character_encoding

## 실행 파일 한글 출력 수정 — 2026-10-01

스크립트 BOM 문제와 별개로 CP949 콘솔에서 UTF-8 실행 파일 출력이 깨지는 문제를 수정했다.
output.c가 콘솔에는 WriteConsoleW, 파일·파이프에는 UTF-8을 사용한다.
시스템 설정과 PowerShell 프로필, 사용자 콘솔 코드 페이지는 변경하지 않는다.

```powershell
.\tests\console.ps1
```

별도 콘솔을 CP949와 UTF-8로 설정하고 실제 화면 버퍼에서 도움말·잘못된 인자·장치 목록의 한글을 검사한다.
총 6개 사례가 통과했고 프로그램 종료 후에도 코드 페이지가 유지됐다.
Windows PowerShell과 PowerShell 7의 기존 CLI 17개, 모의 캡처, 실제 loopback 테스트도 통과했다.
직접 콘솔 출력과 달리 PowerShell 파이프/리디렉션은 셸의 디코딩·저장 규칙이 추가로 적용된다.
파이프 소비자는 UTF-8을 지정해야 한다. 사용자 터미널의 폰트 렌더링은 직접 확인이 필요하다.

## 패킷 캡처 시각 — 2026-10-01

패킷 줄 끝에 `캡처 시각=YYYY-MM-DDTHH:mm:ss.ffffffZ`를 출력한다.
Npcap 헤더의 ts를 사용하며 UTC와 소수점 6자리로 표시한다. 출력 시각을 새로 측정하지 않는다.
실행 시간 제한은 기존 GetTickCount64 기반을 유지한다.
모의 테스트를 15개 시나리오로 확대했고 고정 epoch 1704067200 + 7마이크로초가
`2024-01-01T00:00:00.000007Z`로 출력되는지 검사한다.
잘못된 마이크로초·음수 초에서 오류 반환과 자원 정리를 검사한다.
실시간 테스트는 패킷 5개 각각의 시각 형식과 실제 실행 구간 포함 여부를 확인한다.
Wireshark와 실제 동시 캡처 대조는 미실행이며 방법은 USER_GUIDE.md에 기록했다.

## Wireshark 현지 시간 형식 맞춤 — 2026-10-01

기존 UTC T/Z 표기를 현지 `YYYY-MM-DD HH:mm:ss.ffffff`로 변경했다.
Npcap의 원래 ts는 유지하고 localtime으로 표시만 변환한다.
Wireshark 공식 한국어 번역에서 메뉴명 `보기 → 시간 표시 형식 → 날짜와 시간`, `마이크로초`를 확인했다.
모의 epoch 1704067200 + 7µs를 .NET 현지 시간 변환 결과와 비교해 통과했다.
엄격 빌드·CLI 17개·모의 시나리오 15개·실제 loopback 5개 시각·길이·종료 검증도 통과했다.
Wireshark 화면을 직접 조작하거나 별도 동시 캡처와 측정값을 대조하지는 않았다.
동일 표시 형식은 지원하지만 서로 다른 캡처의 시각이 항상 완전히 같다고 보장하지 않는다.
