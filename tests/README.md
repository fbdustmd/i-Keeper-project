# 테스트

프로젝트 루트에서 실행한다.

```powershell
.\tests\smoke.ps1
.\tests\unit.ps1
```

- smoke.ps1: 경고를 오류로 처리하는 빌드, 기존 도움말과 신규 인자·장치 목록 검증 17개. 실제 캡처는 열지 않는다.
- unit.ps1: 실제 Npcap 대신 모의 함수를 링크한다. 15개 시나리오 및 잘못된 설정·장치를 검증하고 오류 경로에서도 핸들과 필터가 해제되는지 검사한다.

실시간 검증은 따로 실행한다. loopback 인터페이스를 명시적으로 지정해야 한다.

```powershell
.\tests\capture.ps1 -Interface '\Device\NPF_Loopback'
```

capture.ps1은 로컬 UDP 임시 포트로만 실습 데이터를 보내고 해당 포트만 캡처한다.
수·길이, 무트래픽 시간 제한, Ctrl+C, 잘못된 필터 처리를 확인한다.
ctrlc_test.c는 호출자의 콘솔과 분리된 테스트 콘솔에서만 Ctrl+C를 발생시킨다.

SDK 기본 경로는 `.deps/npcap-sdk`다. 자세한 결과와 제한사항은 [테스트 안내](../docs/TESTING.md)에 있다.

콘솔 한글 출력 검증: `.\tests\console.ps1`.
별도 CP949/UTF-8 콘솔 화면에서 도움말·오류·장치 목록의 한글을 확인하며 실제 캡처를 열지 않는다.
