# 콘솔 한글 출력 수정 — 2026-10-01

## 문제와 목표

PowerShell 스크립트의 UTF-8 BOM 수정 이후에도 netsentry.exe 직접 실행 시 한글이 깨졌다.
C 프로그램은 UTF-8 바이트를 출력하지만 CP949 콘솔은 다른 문자 집합으로 해석했다.
기존 smoke는 UTF-8로 파이프를 읽었으므로 실제 콘솔 경로를 검증하지 못했다.

## 구현

- output.h/c에 output_printf를 추가하고 main·capture·모의 테스트의 한글 출력을 연결한다.
- GetConsoleMode로 출력 대상이 콘솔인지 확인한다.
- 콘솔은 UTF-8을 UTF-16으로 변환해 WriteConsoleW로 출력한다.
- 파일·파이프는 UTF-8 바이트를 유지한다. stdout/stderr는 각각 독립적으로 판단한다.
- 가변 길이 출력은 필요한 크기를 계산해 할당하고 해제한다. GCC 형식 검사 속성을 유지한다.
- 컴파일 입력·실행 문자 집합은 UTF-8로 명시한다. ps1의 BOM을 유지한다.
- 사용자 프로필·시스템 로캘·콘솔 코드 페이지는 변경하지 않는다.

## 검증

- [x] Windows PowerShell 및 PowerShell 7의 엄격 빌드·CLI 17개
- [x] CP949/UTF-8 콘솔 화면 버퍼에서 도움말·오류·장치 목록 6개와 코드 페이지 유지 검사
- [x] 모의 캡처 테스트
- [x] loopback 캡처·시간 제한·Ctrl+C·필터 오류 회귀 검증

콘솔 테스트는 별도 숨김 콘솔을 만들고 실제 자식 프로그램의 화면 버퍼를 Unicode로 읽는다.
텍스트를 파이프로만 수집하는 기존 테스트와 구분한다.
모든 터미널 앱의 폰트 표시까지 자동 검증한 것은 아니다.
PowerShell 파이프의 후속 디코딩은 셸 설정에 따라 달라질 수 있으며 소비자는 UTF-8로 읽어야 한다.

근거: https://learn.microsoft.com/en-us/windows/console/writeconsole
