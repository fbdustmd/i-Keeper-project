[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location -LiteralPath $projectRoot
try {
    New-Item -ItemType Directory -Force build | Out-Null
    & gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -save-temps=obj -I include `
        -I .deps/npcap-sdk/Include tests/capture_test.c src/capture.c -o build/capture_test.exe
    if ($LASTEXITCODE -ne 0) { throw '모의 테스트 컴파일 실패' }
    & .\build\capture_test.exe
    if ($LASTEXITCODE -ne 0) { throw '모의 테스트 실패' }
}
finally { Pop-Location }
