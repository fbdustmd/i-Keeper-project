# 헤더 파일

capture.h는 CaptureOptions와 인터페이스 목록·실시간 캡처 함수의 공개 선언을 제공한다.
Npcap 핸들 생성·정리는 capture.c 안에서 맡으며 main.c에는 노출하지 않는다.
후속 모듈은 해당 Phase에서 검증할 기능이 생길 때 추가한다.
