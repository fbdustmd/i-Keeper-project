[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot

try {
    & (Join-Path $projectRoot 'build.ps1') -WarningsAsErrors
    $executable = Join-Path $projectRoot 'build/netsentry.exe'
    $cases = @(
        @{ Name = '인자 없음'; Arguments = ''; Expected = 0 },
        @{ Name = '도움말'; Arguments = '--help'; Expected = 0 },
        @{ Name = '잘못된 인자'; Arguments = '--invalid'; Expected = 1 },
        @{ Name = '추가 인자'; Arguments = '--help extra'; Expected = 1 }
    )

    foreach ($case in $cases) {
        $startInfo = New-Object System.Diagnostics.ProcessStartInfo
        $startInfo.FileName = $executable
        $startInfo.Arguments = $case.Arguments
        $startInfo.UseShellExecute = $false
        $startInfo.CreateNoWindow = $true
        $startInfo.RedirectStandardOutput = $true
        $startInfo.RedirectStandardError = $true
        $startInfo.StandardOutputEncoding = [System.Text.Encoding]::UTF8
        $startInfo.StandardErrorEncoding = [System.Text.Encoding]::UTF8
        $process = New-Object System.Diagnostics.Process
        $process.StartInfo = $startInfo
        try {
            if (-not $process.Start()) { throw 'NetSentry를 실행하지 못했습니다.' }
            $stdoutTask = $process.StandardOutput.ReadToEndAsync()
            $stderrTask = $process.StandardError.ReadToEndAsync()
            if (-not $process.WaitForExit(10000)) {
                $process.Kill()
                $process.WaitForExit()
                throw "$($case.Name): 실행 제한 시간을 초과했습니다."
            }
            $stdout = $stdoutTask.GetAwaiter().GetResult()
            $stderr = $stderrTask.GetAwaiter().GetResult()
            if ($process.ExitCode -ne $case.Expected) {
                throw "$($case.Name): 예상 종료 코드 $($case.Expected), 실제 종료 코드 $($process.ExitCode)."
            }
            if ($case.Expected -eq 0) {
                if ($stdout -notmatch '사용법: netsentry' -or $stderr.Length -ne 0) {
                    throw "$($case.Name): stdout에 사용법이 있어야 하고 stderr는 비어 있어야 합니다."
                }
            }
            elseif ($stderr -notmatch '오류:' -or $stdout.Length -ne 0) {
                throw "$($case.Name): stderr에 오류가 있어야 하고 stdout은 비어 있어야 합니다."
            }
            Write-Output "통과: $($case.Name) (종료 코드 $($process.ExitCode))"
        }
        finally {
            $process.Dispose()
        }
    }
    Write-Output '모든 기본 동작 검증을 통과했습니다.'
    exit 0
}
catch {
    [Console]::Error.WriteLine("기본 동작 검증 실패: $($_.Exception.Message)")
    exit 1
}
