# Windows 개발 환경

## 런타임과 SDK의 차이

Npcap 런타임은 패킷을 수집하는 드라이버와 실행 시 호출하는 DLL이다.
SDK는 C 컴파일에 필요한 헤더와 링크용 라이브러리다. 런타임이 설치되어 있어도 SDK가 없으면 `pcap.h`와 `wpcap.lib`를 찾지 못해 빌드할 수 없다.

프로젝트는 기존 Windows + PowerShell + MSYS2 UCRT64 GCC를 유지한다.
SDK 다운로드·압축 해제는 프로젝트 안의 파일 준비이며 드라이버 설치가 아니다.

## SDK 준비

프로젝트 루트에서 실행한다. 이미 준비되어 있으면 반복할 필요가 없다.

```powershell
New-Item -ItemType Directory -Force .deps | Out-Null
Invoke-WebRequest 'https://npcap.com/dist/npcap-sdk-1.16.zip' -OutFile '.deps/npcap-sdk-1.16.zip'
Expand-Archive '.deps/npcap-sdk-1.16.zip' -DestinationPath '.deps/npcap-sdk'
.\build.ps1 -WarningsAsErrors
```

기본 경로는 `.deps/npcap-sdk/Include/pcap.h`와 `.deps/npcap-sdk/Lib/x64/wpcap.lib`다.
다른 SDK 경로는 `build.ps1 -NpcapSdk <경로>`로 지정할 수 있다. 자동 테스트는 기본 경로를 사용한다.
`.deps/`와 `build/`는 Git 제외 대상이다. SDK와 설치된 DLL을 GitHub에 올리거나 빌드 폴더째 배포하지 않는다.

빌드는 설치된 `%SystemRoot%/System32/Npcap/wpcap.dll`과 `Packet.dll`을 `build/`에 복사한다.
실행 파일 옆의 DLL을 사용하므로 시스템 PATH 영구 변경이 필요 없다. Npcap 업데이트 후 다시 빌드하면 복사본도 갱신된다.
런타임이 없거나 접근 권한이 제한된 PC에서는 환경 준비가 별도로 필요하며 스크립트가 설치·권한 변경을 자동 수행하지 않는다.

## 2026-09-30 실제 확인 결과

- PowerShell 7.6.5, MSYS2 UCRT64 GCC 16.1.0, 대상 `x86_64-w64-mingw32`
- Npcap 서비스 Running, `AdminOnly=0`, `WinPcapCompatible=1`
- `pcap_lib_version()`: Npcap 1.87 / libpcap 1.10.6 (64-bit time_t)
- SDK 1.16의 헤더와 x64 `wpcap.lib`로 최소 프로그램 컴파일·링크·실행 성공
- 장치 열거 성공, 사용자가 승인한 `\Device\NPF_Loopback` 접근 및 캡처 성공
- GCC에서 `Packet.lib`를 직접 링크할 필요는 없었으나 실행 시 Packet.dll은 필요하다.
- 전역 PATH, 드라이버, 관리자 전용 설정은 변경하지 않았다.

다운로드 ZIP의 실제 SHA256은 다음과 같다. 공식 서명 검증을 대신하는 값이 아니라 이번 다운로드를 식별하기 위한 기록이다.

```text
F0A8BE7778EE3AE1B99BBBECB27A3FF0F6C111A4093F1C78C5C5A099607184DB
```

## loopback을 선택한 이유

loopback은 같은 PC 안에서 `127.0.0.1`로 통신하는 경로다.
실시간 테스트는 임시 UDP 포트를 열고 그 포트의 패킷만 BPF로 선택한다.
다른 Wi-Fi·Ethernet 인터페이스는 열지 않으며 외부 서버로 테스트 트래픽을 보내지 않는다.
UDP는 길이를 예측할 수 있는 실습 트래픽 생성에만 사용한다. NetSentry에 UDP 파서를 구현한 것은 아니다.

참고: [Npcap SDK 안내](https://npcap.com/guide/npcap-devguide.html),
[공식 다운로드](https://npcap.com/#download).
