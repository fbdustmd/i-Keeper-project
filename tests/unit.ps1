[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location -LiteralPath $projectRoot
try {
    New-Item -ItemType Directory -Force build | Out-Null
    & gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -save-temps=obj -I include `
        -I .deps/npcap-sdk/Include tests/capture_test.c src/capture.c src/output.c -o build/capture_test.exe
    if ($LASTEXITCODE -ne 0) { throw '모의 테스트 컴파일 실패' }
    $info = [Diagnostics.ProcessStartInfo]::new()
    $info.FileName = Join-Path $projectRoot 'build/capture_test.exe'
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    $info.StandardOutputEncoding = [Text.Encoding]::UTF8
    $info.StandardErrorEncoding = [Text.Encoding]::UTF8
    $process = [Diagnostics.Process]::Start($info)
    try {
        $outputTask = $process.StandardOutput.ReadToEndAsync()
        $errorTask = $process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(10000)) { $process.Kill(); throw '모의 테스트 시간 초과' }
        $output = $outputTask.GetAwaiter().GetResult()
        $errors = $errorTask.GetAwaiter().GetResult()
        if ($process.ExitCode -ne 0) { throw "모의 테스트 실패: $errors" }
        $timestamps = [regex]::Matches($output, '캡처 시각=(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{6})')
        if ($timestamps.Count -ne 4) { throw '예상 패킷 시각 4개가 아닙니다.' }
        $expectedTime = [DateTimeOffset]::FromUnixTimeSeconds(1704067200).LocalDateTime.ToString('yyyy-MM-dd HH:mm:ss', [Globalization.CultureInfo]::InvariantCulture) + '.000007'
        foreach ($timestamp in $timestamps) {
            if ($timestamp.Groups[1].Value -ne $expectedTime) {
                throw '캡처 헤더의 현지 시각 또는 마이크로초 0 채움이 일치하지 않습니다.'
            }
        }
        if ([regex]::Matches($errors, '잘못된 캡처 시각').Count -ne 2) {
            throw '잘못된 시각 오류 검증 실패'
        }
        Write-Output $output
        Write-Output '통과: 고정 캡처 시각과 현지·마이크로초 출력, 잘못된 시각 처리'
    } finally { $process.Dispose() }
}
finally { Pop-Location }
