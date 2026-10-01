[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location -LiteralPath $projectRoot
try {
    & ./build.ps1 -WarningsAsErrors
    & gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -save-temps=obj -municode `
        -finput-charset=UTF-8 -fexec-charset=UTF-8 tests/console_test.c -o build/console_test.exe
    if ($LASTEXITCODE -ne 0) { throw '콘솔 테스트 빌드 실패' }
    # 별도 콘솔을 만드는 도구를 창 없이 실행한다.
    $info = [Diagnostics.ProcessStartInfo]::new()
    $info.FileName = Join-Path $projectRoot 'build/console_test.exe'
    $info.Arguments = '"' + (Join-Path $projectRoot 'build/netsentry.exe') + '"'
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $process = [Diagnostics.Process]::Start($info)
    try {
        if (-not $process.WaitForExit(70000)) { $process.Kill(); throw '콘솔 테스트 시간 초과' }
        if ($process.ExitCode -ne 0) { throw '콘솔 문자 검증 실패' }
    } finally { $process.Dispose() }
    Write-Output '통과: CP949·UTF-8 콘솔의 도움말·오류·목록 한글과 코드 페이지 유지 (6개)'
} finally { Pop-Location }
