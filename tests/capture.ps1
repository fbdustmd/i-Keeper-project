[CmdletBinding()]
param([Parameter(Mandatory)][string]$Interface)
$ErrorActionPreference = 'Stop'
if ($Interface -ne '\Device\NPF_Loopback') {
    throw '이 실습 테스트는 명시적으로 선택한 Npcap loopback만 지원합니다.'
}
$projectRoot = Split-Path -Parent $PSScriptRoot
Push-Location -LiteralPath $projectRoot
$receiver = $null
$sender = $null
function Invoke-CaptureCase {
    param([string]$File, [string]$CommandArguments, [scriptblock]$OnReady,
          [int]$Expected = 0)
    $info = [System.Diagnostics.ProcessStartInfo]::new()
    $info.FileName = $File
    $info.Arguments = $CommandArguments
    $info.UseShellExecute = $false
    $info.CreateNoWindow = $true
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    $info.StandardOutputEncoding = [Text.Encoding]::UTF8
    $info.StandardErrorEncoding = [Text.Encoding]::UTF8
    $process = [System.Diagnostics.Process]::new()
    $process.StartInfo = $info
    $timer = [Diagnostics.Stopwatch]::StartNew()
    try {
        if (-not $process.Start()) { throw '실행 실패' }
        $errorTask = $process.StandardError.ReadToEndAsync()
        $prefix = ''
        if ($OnReady) {
            foreach ($index in 1..2) {
                $lineTask = $process.StandardOutput.ReadLineAsync()
                if (-not $lineTask.Wait(5000)) { throw '캡처 준비 시간 초과' }
                $line = $lineTask.GetAwaiter().GetResult()
                if ($null -eq $line) { throw '캡처 준비 전에 종료됨' }
                $prefix += $line + "`n"
            }
            if ($prefix -notmatch '링크 타입:') { throw '캡처 준비 출력 없음' }
            & $OnReady
        }
        $outputTask = $process.StandardOutput.ReadToEndAsync()
        if (-not $process.WaitForExit(10000)) { throw '종료 제한 시간 초과' }
        $output = $prefix + $outputTask.GetAwaiter().GetResult()
        $errors = $errorTask.GetAwaiter().GetResult()
        if ($process.ExitCode -ne $Expected) { throw "예상 종료 코드 $Expected, 실제 $($process.ExitCode): $errors" }
        if ($Expected -eq 0 -and $errors) { throw $errors }
        if ($output -notmatch '캡처 자원 정리 완료') { throw '자원 정리 결과 없음' }
        return @{ Output = $output; Error = $errors; Seconds = $timer.Elapsed.TotalSeconds }
    }
    finally {
        if ($process.Id -and -not $process.HasExited) { $process.Kill(); $process.WaitForExit() }
        $process.Dispose()
    }
}
try {
    & ./build.ps1 -WarningsAsErrors
    & gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -save-temps=obj -municode `
        tests/ctrlc_test.c -o build/ctrlc_test.exe
    if ($LASTEXITCODE -ne 0) { throw 'Ctrl+C 검증 도구 빌드 실패' }
    $exe = Join-Path $projectRoot 'build/netsentry.exe'
    $receiver = [Net.Sockets.UdpClient]::new([Net.IPEndPoint]::new([Net.IPAddress]::Loopback, 0))
    $port = $receiver.Client.LocalEndPoint.Port
    $sender = [Net.Sockets.UdpClient]::new()
    $sender.Connect([Net.IPAddress]::Loopback, $port)
    $arguments = "--interface `"$Interface`" --filter `"ip and udp and dst port $port`""
    $traffic = Invoke-CaptureCase $exe "$arguments --count 5 --duration 5" {
        $payload = [Text.Encoding]::ASCII.GetBytes('NETSENTRY-LOCAL-TEST-0123456789A')
        if ($payload.Length -ne 32) { throw '실습 데이터 길이 오류' }
        foreach ($index in 1..5) { [void]$sender.Send($payload, $payload.Length) }
    }
    $packets = [regex]::Matches($traffic.Output, '패킷 #\d+ 캡처 길이=(\d+) 원래 길이=(\d+)')
    if ($packets.Count -ne 5 -or $traffic.Output -notmatch '패킷 수 제한, 패킷=5') { throw $traffic.Output }
    if ($traffic.Output -notmatch '링크 타입: 0 \(NULL\)') { throw '예상 loopback 링크 타입과 다릅니다.' }
    foreach ($packet in $packets) {
        # DLT_NULL 4 + IPv4 20 + UDP 8 + 실습 데이터 32 = 64바이트
        if ($packet.Groups[1].Value -ne '64' -or $packet.Groups[2].Value -ne '64') { throw '패킷 길이 불일치' }
    }
    if ($traffic.Output -match 'NETSENTRY-LOCAL-TEST') { throw '원본 데이터가 출력됨' }
    Write-Output '통과: 실습 패킷 5개, 각 64바이트, 원본 데이터 출력 없음'
    $idle = Invoke-CaptureCase $exe "$arguments --count 1 --duration 2" $null
    if ($idle.Output -notmatch '시간 제한, 패킷=0' -or $idle.Seconds -lt 2 -or $idle.Seconds -gt 6) { throw $idle.Output }
    Write-Output ('통과: 무트래픽 시간 제한 ({0:F2}초)' -f $idle.Seconds)
    $cancel = Invoke-CaptureCase (Join-Path $projectRoot 'build/ctrlc_test.exe') "`"$Interface`" $port" $null
    if ($cancel.Output -notmatch '종료 요청, 패킷=0' -or $cancel.Seconds -gt 7) { throw $cancel.Output }
    Write-Output ('통과: 무트래픽 Ctrl+C 종료 ({0:F2}초)' -f $cancel.Seconds)
    $badFilter = Invoke-CaptureCase $exe "--interface `"$Interface`" --filter `"invalid ( filter`"" $null 1
    if ($badFilter.Error -notmatch '필터 문법 오류') { throw '잘못된 필터 오류 없음' }
    Write-Output '통과: 잘못된 필터 오류 및 자원 정리'
}
finally {
    if ($sender) { $sender.Dispose() }
    if ($receiver) { $receiver.Dispose() }
    Pop-Location
}
