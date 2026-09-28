[CmdletBinding()]
param([switch]$Clean, [switch]$WarningsAsErrors)

$ErrorActionPreference = 'Stop'
$buildDirectory = Join-Path $PSScriptRoot 'build'
$executable = Join-Path $buildDirectory 'netsentry.exe'

if ($Clean) {
    foreach ($name in @('netsentry.exe', 'netsentry-main.i', 'netsentry-main.s', 'netsentry-main.o')) {
        $artifact = Join-Path $buildDirectory $name
        if (Test-Path -LiteralPath $artifact) {
            Remove-Item -LiteralPath $artifact
        }
    }
    return
}

$compiler = Get-Command gcc -CommandType Application -ErrorAction Stop
New-Item -ItemType Directory -Path $buildDirectory -Force | Out-Null
Push-Location -LiteralPath $PSScriptRoot
try {
    $extraFlags = @()
    if ($WarningsAsErrors) { $extraFlags += '-Werror' }
    # 한글 TEMP 경로 문제를 피하도록 중간 파일을 실행 파일 옆에 저장한다.
    & $compiler.Source '-std=c11' '-Wall' '-Wextra' '-Wpedantic' '-save-temps=obj' `
        @extraFlags 'src/main.c' '-o' 'build/netsentry.exe'
    if ($LASTEXITCODE -ne 0) {
        throw "GCC 빌드 실패: 종료 코드 $LASTEXITCODE."
    }
    if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
        throw 'GCC가 netsentry.exe를 생성하지 않았습니다.'
    }
}
finally {
    Pop-Location
}
Write-Output "빌드 완료: $executable"
