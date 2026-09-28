[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot

try {
    & (Join-Path $projectRoot 'build.ps1') -WarningsAsErrors
    $executable = Join-Path $projectRoot 'build/netsentry.exe'
    $cases = @(
        @{ Name = 'no arguments'; Arguments = ''; Expected = 0 },
        @{ Name = 'help'; Arguments = '--help'; Expected = 0 },
        @{ Name = 'invalid argument'; Arguments = '--invalid'; Expected = 1 },
        @{ Name = 'extra argument'; Arguments = '--help extra'; Expected = 1 }
    )

    foreach ($case in $cases) {
        $startInfo = New-Object System.Diagnostics.ProcessStartInfo
        $startInfo.FileName = $executable
        $startInfo.Arguments = $case.Arguments
        $startInfo.UseShellExecute = $false
        $startInfo.CreateNoWindow = $true
        $startInfo.RedirectStandardOutput = $true
        $startInfo.RedirectStandardError = $true
        $process = New-Object System.Diagnostics.Process
        $process.StartInfo = $startInfo
        try {
            if (-not $process.Start()) { throw 'Could not start NetSentry.' }
            $stdoutTask = $process.StandardOutput.ReadToEndAsync()
            $stderrTask = $process.StandardError.ReadToEndAsync()
            if (-not $process.WaitForExit(10000)) {
                $process.Kill()
                $process.WaitForExit()
                throw "$($case.Name): executable timed out."
            }
            $stdout = $stdoutTask.GetAwaiter().GetResult()
            $stderr = $stderrTask.GetAwaiter().GetResult()
            if ($process.ExitCode -ne $case.Expected) {
                throw "$($case.Name): expected exit $($case.Expected), got $($process.ExitCode)."
            }
            if ($case.Expected -eq 0) {
                if ($stdout -notmatch 'Usage: netsentry' -or $stderr.Length -ne 0) {
                    throw "$($case.Name): expected usage on stdout and empty stderr."
                }
            }
            elseif ($stderr -notmatch 'Error:' -or $stdout.Length -ne 0) {
                throw "$($case.Name): expected error on stderr and empty stdout."
            }
            Write-Output "PASS: $($case.Name) (exit $($process.ExitCode))"
        }
        finally {
            $process.Dispose()
        }
    }
    Write-Output 'All smoke tests passed.'
    exit 0
}
catch {
    [Console]::Error.WriteLine("Smoke test failed: $($_.Exception.Message)")
    exit 1
}
