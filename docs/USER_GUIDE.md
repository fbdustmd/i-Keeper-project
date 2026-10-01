# NetSentry 명령어 사용설명서

현재 Phase 1 구현 기준 · 작성일: 2026-10-01

이 문서는 Windows와 PowerShell에서 NetSentry를 직접 빌드하고 테스트하는 방법을 설명합니다.
예제는 프로젝트 루트에서 실행합니다. `PS C:\...>` 같은 프롬프트는 입력하지 않고 코드 블록의 명령만 복사하세요.

## 목차

1. [현재 가능한 기능](#1-현재-가능한-기능)
2. [처음 실행하는 순서](#2-처음-실행하는-순서)
3. [빌드 명령](#3-빌드-명령)
4. [실행 옵션 전체 목록](#4-실행-옵션-전체-목록)
5. [옵션별 상세 설명](#5-옵션별-상세-설명)
6. [직접 해 보는 loopback 실습](#6-직접-해-보는-loopback-실습)
7. [출력 읽는 방법](#7-출력-읽는-방법)
8. [자동 테스트](#8-자동-테스트)
9. [출력 저장과 한글](#9-출력-저장과-한글)
10. [오류 해결](#10-오류-해결)
11. [자주 쓰는 명령 모음](#11-자주-쓰는-명령-모음)
12. [관련 문서와 코드](#12-관련-문서와-코드)

## 1. 현재 가능한 기능

NetSentry는 선택한 인터페이스에서 실시간 패킷을 받아 번호·길이·링크 타입·캡처 시각을 출력합니다.
패킷 수, 실행 시간, Ctrl+C로 캡처를 종료할 수 있습니다.
BPF 캡처 필터로 수집 대상을 좁힐 수 있습니다.

현재 지원하지 않는 기능:

- Ethernet·IPv4·TCP 헤더의 직접 분석과 주소·포트 출력
- PCAP 파일 읽기·쓰기
- TCP 연결 관리와 스트림 재조립
- HTTP 분석과 민감정보 탐지

필터에 `tcp`를 사용할 수 있는 것은 Npcap의 필터 기능 덕분입니다. NetSentry의 TCP 파서가 구현된 것은 아닙니다.
현재 출력에는 패킷 원본 데이터가 들어가지 않습니다.

## 2. 처음 실행하는 순서

### 2.1 프로젝트 폴더로 이동

```powershell
Set-Location -LiteralPath 'C:\Users\류연승\Documents\ChatGPT\키퍼 프로젝트\NetSentry'
```

다른 PC에서는 실제 프로젝트 경로로 바꾸세요. 경로에 공백이 있으므로 따옴표를 유지합니다.
`Set-Location`은 현재 작업 폴더를 바꾸는 PowerShell 명령입니다.

### 2.2 환경 확인

```powershell
$PSVersionTable.PSVersion
gcc --version
```

주 개발환경은 PowerShell 7과 MSYS2 UCRT64 x64 GCC입니다.
Windows PowerShell 5.1에서도 한글 인코딩 수정 후 빌드·기본 테스트·콘솔 테스트를 검증했습니다.
실시간 테스트는 PowerShell 7에서 검증했습니다.

Npcap 런타임과 SDK가 필요합니다. 현재 개발 PC에는 준비되어 있습니다.
GitHub에서 새로 받은 프로젝트에는 SDK와 DLL이 포함되지 않으므로 [개발 환경](ENVIRONMENT.md)을 먼저 확인하세요.

### 2.3 스크립트 실행이 차단될 때만 적용

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy RemoteSigned
```

확인 질문이 나오면 내용을 확인하고 `Y`를 입력합니다.
현재 PowerShell 세션에만 적용되며 영구적인 시스템 설정 변경은 아닙니다.
조직의 그룹 정책으로 차단된 환경은 이 명령보다 그룹 정책이 우선합니다.

### 2.4 빌드와 기본 확인

```powershell
.\build.ps1 -WarningsAsErrors
.\build\netsentry.exe --help
.\build\netsentry.exe --list
```

`빌드 완료:`가 나오면 실행 파일 생성에 성공한 것입니다.
`--help`는 도움말, `--list`는 캡처 장치 목록을 표시합니다. 이 두 명령은 실제 캡처를 시작하지 않습니다.

## 3. 빌드 명령

### 경고를 오류로 처리하는 빌드

```powershell
.\build.ps1 -WarningsAsErrors
```

평소에는 이 명령을 사용합니다. 컴파일 경고가 있으면 빌드 실패로 처리합니다.
C11과 `-Wall -Wextra -Wpedantic -Werror`를 사용합니다.

### 기본 빌드

```powershell
.\build.ps1
```

경고를 출력하되 모든 경고를 오류로 취급하지는 않습니다.

### 빌드 산출물 정리 후 다시 빌드

```powershell
.\build.ps1 -Clean
.\build.ps1 -WarningsAsErrors
```

`-Clean`은 NetSentry 실행 파일, 해당 컴파일 중간 파일, 복사된 Npcap DLL을 제거합니다.
소스 코드와 SDK는 지우지 않습니다. 테스트 실행 파일 등 build/의 모든 파일을 지우는 명령은 아닙니다.

### SDK 경로를 직접 지정

```powershell
.\build.ps1 -WarningsAsErrors -NpcapSdk '.deps/npcap-sdk'
```

기본 SDK 경로와 같은 예입니다. 다른 위치에 SDK가 있다면 그 경로로 바꿉니다.
해당 폴더 아래에 `Include/pcap.h`와 `Lib/x64/wpcap.lib`가 있어야 합니다.
테스트 스크립트들은 기본 SDK 위치를 사용합니다.

빌드 결과는 `build/netsentry.exe`입니다. 함께 복사된 `wpcap.dll`과 `Packet.dll`도 실행에 필요하므로 실행 파일만 따로 옮기지 마세요.

## 4. 실행 옵션 전체 목록

전체 형식은 다음과 같습니다. 대괄호는 생략 가능한 옵션이라는 설명 표기이며 실제로 입력하지 않습니다.

```text
netsentry [--help]
netsentry --list
netsentry --interface NAME [--count N] [--duration SECONDS] [--filter BPF]
```

| 옵션 | 역할 | 기본값 | 조건 |
|---|---|---|---|
| 옵션 없음 | 도움말 출력 | 해당 없음 | 캡처하지 않음 |
| `--help` | 도움말 출력 | 해당 없음 | 단독 사용 |
| `--list` | 인터페이스 목록 출력 | 해당 없음 | 단독 사용 |
| `--interface NAME` | 캡처할 장치 지정 | 없음 | 캡처 시 필수 |
| `--count N` | 최대 패킷 수 | 100개 | 1~1000000 |
| `--duration SECONDS` | 캡처 시간 제한 | 30초 | 1~86400 |
| `--filter BPF` | 캡처 조건 지정 | 필터 없음 | 비어 있지 않은 표현식, 현재 코드상 최대 4096바이트 |

옵션 이름은 대소문자를 구분합니다. `--count=5`, `-c`, `--version`은 지원하지 않습니다.
옵션과 값은 공백으로 구분하며, 같은 옵션을 두 번 지정할 수 없습니다.
캡처 옵션들의 순서는 바꿀 수 있습니다.

## 5. 옵션별 상세 설명

### 5.1 도움말: --help

```powershell
.\build\netsentry.exe --help
```

옵션 없이 실행해도 같은 도움말을 표시합니다.

```powershell
.\build\netsentry.exe
```

### 5.2 장치 확인: --list

```powershell
.\build\netsentry.exe --list
```

장치 이름과 설명이 출력됩니다. `--interface`에는 설명이 아니라 장치 이름을 복사합니다.
예를 들어 이 PC의 loopback 이름은 `\Device\NPF_Loopback`입니다.
목록 순서나 장치 이름을 다른 PC에서도 같다고 가정하면 안 됩니다.

### 5.3 장치 선택: --interface

```powershell
.\build\netsentry.exe --interface '\Device\NPF_Loopback'
```

지정한 장치에서 최대 100개 또는 30초까지 캡처합니다.
loopback은 같은 PC 내부의 통신 경로입니다. 브라우저의 일반 인터넷 통신이 모두 여기에서 보이는 것은 아닙니다.
Wi-Fi/Ethernet을 사용하려면 `--list`에서 실제 이름을 확인하고 분석 권한이 있는 인터페이스를 지정하세요.
현재 실시간 자동 검증은 loopback에서만 수행했습니다.

### 5.4 패킷 수 제한: --count

```powershell
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --count 5
```

최대 5개를 받습니다. 기본 시간 제한 30초도 그대로 적용되므로 5개가 오지 않아도 시간이 지나면 종료합니다.
필터를 사용하면 프로그램에 전달되어 처리된 패킷을 셉니다. 네트워크 전체 패킷 수나 전송 손실 통계가 아닙니다.

다음 명령은 인터페이스가 빠졌으므로 정상적으로 오류가 발생합니다.

```powershell
# 의도적인 잘못된 사용 예
.\build\netsentry.exe --count 5
```

### 5.5 시간 제한: --duration

```powershell
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --duration 10
```

최대 10초 동안 수집합니다. 기본 개수 제한 100개에 먼저 도달하면 더 일찍 종료합니다.
시간은 장치를 열고 설정한 뒤 캡처 루프에 들어가는 시점부터 측정합니다.
장치 열기 지연이나 출력 대상의 정체까지 포함한 프로세스 전체 제한 시간은 아닙니다.

두 제한을 함께 지정할 수 있습니다.

```powershell
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --count 5 --duration 10
```

| 상황 | 종료 이유 |
|---|---|
| 2초 만에 5개 수집 | 패킷 수 제한 |
| 10초 동안 2개 수집 | 시간 제한 |
| 10초 동안 0개 수집 | 시간 제한 |
| 도중에 Ctrl+C | 종료 요청 |

0, 음수, 소수, 무제한 설정은 지원하지 않습니다.

### 5.6 필터: --filter

```powershell
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --filter 'icmp' --duration 10
```

BPF는 Npcap이 해석하는 캡처 조건입니다. 공백이 포함된 표현식은 반드시 따옴표로 묶습니다.

| 표현식 | 선택할 트래픽 |
|---|---|
| `'icmp'` | IPv4 ICMP 트래픽, 로컬 ping 실습에 사용 |
| `'ip and tcp'` | IPv4 TCP 트래픽 |
| `'ip and tcp port 80'` | IPv4 TCP 중 출발지 또는 목적지 포트가 80인 트래픽 |
| `'ip and udp and dst port 50000'` | IPv4 UDP 중 목적지 포트가 50000인 트래픽 |

조건에 맞는 트래픽이 실제로 있어야 패킷이 출력됩니다. 필터가 트래픽을 만들어 주지는 않습니다.
포트가 80이라고 반드시 HTTP인 것은 아니며, 현재 프로그램은 HTTP 내용을 분석하지 않습니다.
문법이 잘못되면 필터 오류를 출력하고 종료합니다. 위 표현식 모두의 실제 트래픽을 검증한 것은 아닙니다.

### 5.7 수동 종료: Ctrl+C

캡처 중인 창에 포커스를 두고 Ctrl+C를 누릅니다. 문자열 `Ctrl+C`를 명령어로 입력하는 것이 아닙니다.

정상 종료 시 다음과 같은 메시지가 나옵니다.

```text
종료: 종료 요청, 패킷=0
캡처 자원 정리 완료
```

패킷 수는 실제 수집량에 따라 달라집니다. 트래픽이 없어도 종료 요청을 확인합니다.
터미널 창을 강제로 닫거나 작업 관리자에서 프로세스를 종료하는 것은 이 정상 종료 절차와 다릅니다.

## 6. 직접 해 보는 loopback 실습

### 6.1 첫 번째 PowerShell 창: 캡처 준비

프로젝트 루트에서 실행합니다.

```powershell
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --filter 'icmp' --count 4 --duration 20
```

`캡처 시작`과 `링크 타입`이 표시될 때까지 기다립니다.

### 6.2 두 번째 PowerShell 창: 테스트 트래픽 생성

```powershell
ping -4 127.0.0.1 -n 4
```

같은 PC로 IPv4 ping을 보냅니다. 외부 서버 주소를 사용할 필요가 없습니다.
첫 번째 창에서 패킷 번호와 길이가 출력되는지 확인합니다.
ping에는 요청과 응답이 있으므로 ping 횟수와 캡처 패킷 수가 같지 않을 수 있습니다.

### 6.3 예상 종료

첫 번째 창이 4개를 수집했다면 다음과 같이 끝납니다.

```text
종료: 패킷 수 제한, 패킷=4
캡처 자원 정리 완료
```

트래픽을 늦게 만들면 시간 제한으로 먼저 종료할 수 있습니다. 이 경우 캡처 명령을 다시 실행하세요.

### 6.4 트래픽 없이 종료 확인

아래 명령을 실행하고 별도로 ping을 보내지 않습니다.

```powershell
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --filter 'icmp' --count 100 --duration 3
```

일반적으로 약 3초 뒤 시간 제한으로 끝납니다. 다른 프로그램의 ICMP 트래픽이 있으면 패킷이 보일 수도 있습니다.
정확히 통제된 무트래픽 검증은 `tests/capture.ps1`이 임시 포트 필터로 수행합니다.

## 7. 출력 읽는 방법

다음은 출력 형식을 설명하기 위한 예이며 실제 길이는 트래픽마다 달라집니다.

```text
캡처 시작: \Device\NPF_Loopback
링크 타입: 0 (NULL)
패킷 #1 캡처 길이=64 원래 길이=64 캡처 시각=2026-10-01 12:25:10.123456
종료: 패킷 수 제한, 패킷=1
캡처 자원 정리 완료
```

| 항목 | 의미 |
|---|---|
| 캡처 시작 | 선택한 장치에서 캡처 설정을 완료함 |
| 링크 타입 | 패킷 맨 앞의 헤더 형식 |
| 패킷 #1 | 이번 실행에서 처리한 첫 패킷 |
| 캡처 길이 | 실제로 접근 가능한 데이터 길이, caplen |
| 원래 길이 | Npcap이 보고한 원래 패킷 길이, len |
| 캡처 시각 | Npcap 패킷 헤더의 타임스탬프. PC 현지 날짜·시간과 마이크로초 6자리 |
| 종료 | 수 제한·시간 제한·종료 요청·캡처 오류 중 종료 이유 |
| 자원 정리 완료 | 캡처 정리 경로를 실행함. 성공 여부는 오류 메시지와 종료 코드도 함께 확인 |

`NULL`은 여기서 빈 포인터가 아니라 `DLT_NULL`이라는 링크 타입 이름입니다.
이 PC의 loopback 형식이며 Ethernet 헤더로 해석하면 안 됩니다.

`캡처 길이=100 원래 길이=1500`이라면 캡처된 데이터는 100바이트뿐입니다.
현재는 길이 차이를 표시하고, 이후 파서는 caplen을 기준으로 읽기 범위를 검사하게 됩니다.

실행 직후 종료 코드를 확인할 수 있습니다.

```powershell
$LASTEXITCODE
```

- `0`: 정상 종료. 도움말·목록·시간 제한·수 제한·Ctrl+C 포함
- `1`: 프로그램이 처리한 인자·캡처 오류

DLL 로딩 실패나 강제 종료 같은 OS 수준 오류는 다른 코드가 나올 수 있습니다.

### Wireshark와 캡처 시각 비교

시각은 `2026-10-01 12:25:10.123456` 형식입니다.
PC의 현지 시간대를 사용합니다. 한국 시간대로 설정한 PC에서는 한국 시간으로 표시됩니다. 이전 UTC의 T/Z 표기는 제거했습니다.
소수점 6자리는 마이크로초 표기이며, 실제 측정 정확도가 1마이크로초라는 보장은 아닙니다.
프로그램이 화면에 출력하는 현재 시간이 아니라 Npcap이 패킷에 붙인 `header->ts`를 그대로 변환합니다.

1. Wireshark와 NetSentry에서 같은 인터페이스를 선택합니다. 로컬 실습은 Npcap loopback을 사용합니다.
2. 양쪽 캡처 필터를 `icmp`로 맞춥니다. Wireshark의 캡처 필터와 캡처 후 표시 필터는 별개입니다.
3. Wireshark의 **보기 → 시간 표시 형식 → 날짜와 시간(1970-01-01 01:02:03.123456)**을 선택합니다. **UTC 날짜와 시간**이 아닌 일반 **날짜와 시간** 항목입니다.
4. 다시 **보기 → 시간 표시 형식 → 마이크로초**를 선택합니다. 처음 패킷부터의 상대 시간으로 표시하면 NetSentry의 절대 시각과 바로 비교할 수 없습니다.
5. 양쪽 캡처를 시작한 뒤 다른 창에서 `ping -4 127.0.0.1 -n 4`를 실행합니다.
6. 캡처 시각, 패킷 길이, 발생 순서를 함께 비교합니다. Wireshark의 Frame 상세에서 원래 길이와 캡처 길이를 구분해 확인할 수 있습니다.

설정 후 Wireshark와 NetSentry는 같은 PC에서 현지 날짜·시간과 소수점 6자리라는 동일한 표시 형식을 사용합니다.
별도 캡처의 측정값까지 완전히 일치한다는 뜻은 아닙니다. 현재 패킷 내용·주소·포트는 출력하지 않으므로 시각과 길이가 같은 패킷을 항상 유일하게 구분할 수도 없습니다.

실행 예:

```powershell
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --filter 'icmp' --count 8 --duration 20
```

각 프로그램의 캡처 시작 시점이 다르므로 패킷 번호는 일치하지 않을 수 있습니다.
시간과 길이가 같다는 사실만으로 패킷의 유일한 식별자가 되는 것도 아닙니다.
캡처 경로·설정에 따라 타임스탬프가 다를 수 있어 무조건 완전히 같아야 한다고 판단하지 않습니다.
현재 프로젝트는 실제 Wireshark와 동시 대조까지 자동 검증하지 않았습니다.

참고: [Wireshark 시간 표시](https://www.wireshark.org/docs/wsug_html_chunked/ChWorkTimeFormatsSection.html),
[Npcap 타임스탬프](https://npcap.com/guide/wpcap/pcap-tstamp.html).
## 8. 자동 테스트

| 명령 | 확인하는 내용 | 실제 캡처 |
|---|---|---|
| `.\tests\smoke.ps1` | 엄격 빌드와 CLI 17개 | 없음, 목록 조회만 수행 |
| `.\tests\unit.ps1` | 모의 캡처 15개 시나리오와 잘못된 설정·장치, 자원 해제 | 없음 |
| `.\tests\console.ps1` | CP949/UTF-8 콘솔 한글 6개 사례 | 없음 |
| `.\tests\capture.ps1 -Interface '\Device\NPF_Loopback'` | 패킷 수·길이·시간·Ctrl+C·필터 오류 | 있음, 지정한 loopback만 사용 |

다음 순서로 하나씩 실행하고 결과를 확인하세요.

```powershell
.\tests\smoke.ps1
.\tests\unit.ps1
.\tests\console.ps1
.\tests\capture.ps1 -Interface '\Device\NPF_Loopback'
```

모의 테스트는 일부러 실패 상황을 만듭니다. 중간에 `오류:`가 출력될 수 있으며 마지막 통과 결과를 확인해야 합니다.
실시간 테스트는 임시 UDP 포트로 32바이트 데이터 5개를 보내고 각 캡처가 64바이트인지 검사합니다.
이 스크립트는 loopback만 허용합니다. 다른 장치의 일반 캡처는 실행 파일의 `--interface`로 지정합니다.

스크립트 전체의 성공 여부를 별도 PowerShell 프로세스 종료 코드로 확인하려면 다음처럼 실행합니다.

```powershell
pwsh -NoProfile -File .\tests\smoke.ps1
$LASTEXITCODE
```

`pwsh`가 없다면 Windows PowerShell의 `powershell.exe`를 사용할 수 있습니다.
직접 호출한 일반 ps1 뒤의 `$LASTEXITCODE`는 마지막 외부 프로그램의 값일 수 있으므로 모든 스크립트의 종합 결과라고 가정하지 마세요.

## 9. 출력 저장과 한글

직접 콘솔에 출력할 때는 Unicode를 사용합니다. 시스템 로캘이나 `chcp`를 바꿀 필요가 없습니다.
실행 파일이 파일·파이프로 출력할 때는 UTF-8을 사용합니다.
PowerShell의 `>`와 파이프는 버전에 따라 디코딩·저장 규칙이 달라질 수 있습니다.

셸의 문자 재해석 없이 도움말을 파일에 저장하려면 프로젝트 루트에서 다음 예제를 사용할 수 있습니다.
같은 이름의 파일이 있다면 덮어씁니다.

```powershell
cmd.exe /d /c '.\build\netsentry.exe --help > build\help.txt 2> build\error.txt'
Get-Content -LiteralPath '.\build\help.txt' -Encoding UTF8
```

이 예제는 텍스트 출력 저장이며 PCAP 저장 기능이 아닙니다.
`build/`는 Git 제외 폴더입니다.

## 10. 오류 해결

### 스크립트를 실행할 수 없습니다 / PSSecurityException

현재 창에만 실행 정책을 적용하고 다시 실행합니다.

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy RemoteSigned
.\build.ps1 -WarningsAsErrors
```

계속 실패하면 `Get-ExecutionPolicy -List`로 정책을 확인합니다.

### 한글이 깨지거나 smoke.ps1에서 구문 오류가 발생함

ps1 파일은 UTF-8 BOM으로 유지해야 합니다. 최신 파일을 사용하고 다시 빌드합니다.

```powershell
.\build.ps1 -WarningsAsErrors
.\build\netsentry.exe --help
.\tests\console.ps1
```

소스만 수정하고 다시 빌드하지 않으면 기존 실행 파일의 출력 문제가 남습니다.
최신 실행 파일도 직접 콘솔에서 깨진다면 PowerShell 버전과 오류 화면을 함께 확인해야 합니다.
폰트의 글리프 누락과 문자 인코딩 오류는 별개의 문제입니다.

### gcc를 찾을 수 없음

```powershell
Get-Command gcc
```

현재 셸에서 MSYS2 UCRT64 GCC에 접근할 수 있어야 합니다.
개발 도구 경로는 [개발 환경](ENVIRONMENT.md)을 기준으로 확인합니다.

### Npcap SDK가 없습니다

`.deps/npcap-sdk/Include/pcap.h`와 `.deps/npcap-sdk/Lib/x64/wpcap.lib`를 확인합니다.
SDK는 Git에 포함되지 않습니다. 준비 방법은 [개발 환경](ENVIRONMENT.md)에 있습니다.

### wpcap.dll 또는 Packet.dll을 찾을 수 없음

Npcap 런타임이 설치되어 있고 빌드가 끝까지 성공했는지 확인합니다.
다시 빌드하면 설치된 Npcap DLL을 build/로 복사합니다.
DLL 로딩에 실패하면 프로그램 시작 전 오류이므로 NetSentry의 한국어 오류 처리가 실행되지 않을 수 있습니다.

### 인자·범위·중복 옵션을 확인하세요

다음은 의도적인 잘못된 예입니다.

```powershell
.\build\netsentry.exe --count 5
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --count 0
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --duration 1.5
.\build\netsentry.exe --interface '\Device\NPF_Loopback' --count 1 --count 2
.\build\netsentry.exe --list --count 5
```

각각 인터페이스 누락, 0, 소수, 중복 옵션, 목록 명령에 캡처 옵션을 섞은 경우입니다.

### 지정한 인터페이스가 없습니다

`--list`를 다시 실행하고 장치 이름을 정확히 복사합니다. 설명 문구나 목록 번호를 넣지 않습니다.

### 인터페이스 열기 실패

```powershell
Get-Service npcap
```

장치 존재 여부, Npcap 서비스 상태, 설치 시 지정한 접근 권한을 확인합니다.
목록에 보이는 장치라도 실제 열기에 실패할 수 있습니다. 권한을 무조건 높이기 전에 출력된 원인을 확인하세요.

### 아무 패킷도 나오지 않음

- 선택한 인터페이스로 실제 트래픽이 흐르는지 확인합니다.
- 필터 조건과 발생시킨 트래픽이 일치하는지 확인합니다.
- loopback에서는 `ping -4 127.0.0.1`로 테스트할 수 있습니다.
- 다른 창에서 트래픽을 만들기 전에 캡처가 시작됐는지 확인합니다.

패킷 0개로 시간 제한 종료하는 것은 그 자체로 오류가 아닙니다.

## 11. 자주 쓰는 명령 모음

| 목적 | 명령 |
|---|---|
| 빌드 | `.\build.ps1 -WarningsAsErrors` |
| 도움말 | `.\build\netsentry.exe --help` |
| 장치 확인 | `.\build\netsentry.exe --list` |
| loopback 최대 5개/10초 | `.\build\netsentry.exe --interface '\Device\NPF_Loopback' --count 5 --duration 10` |
| 로컬 ping 캡처 | `.\build\netsentry.exe --interface '\Device\NPF_Loopback' --filter 'icmp' --duration 20` |
| 다른 창에서 ping | `ping -4 127.0.0.1 -n 4` |
| 실행 파일 종료 코드 | `$LASTEXITCODE` |
| 한글 출력 검증 | `.\tests\console.ps1` |
| 실제 캡처 자동 검증 | `.\tests\capture.ps1 -Interface '\Device\NPF_Loopback'` |

## 12. 관련 문서와 코드

- [프로젝트 소개](../README.md)
- [개발 환경과 SDK](ENVIRONMENT.md)
- [테스트 결과와 미검증 범위](TESTING.md)
- [향후 로드맵](ROADMAP.md)
- [기존 캡처 코드 검토](LEGACY_CAPTURE_REVIEW.md)
- [명령어 처리 코드](../src/main.c)
- [캡처 코드](../src/capture.c)
- [한글 출력 코드](../src/output.c)
