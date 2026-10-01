# Windows + MSYS2 UCRT64용 PowerShell 빌드 진입점
.PHONY: all test clean
all:
	pwsh -NoProfile -File ./build.ps1 -WarningsAsErrors

test:
	pwsh -NoProfile -File ./tests/smoke.ps1

clean:
	pwsh -NoProfile -File ./build.ps1 -Clean
