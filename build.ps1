[CmdletBinding()]
param([switch]$Clean, [switch]$WarningsAsErrors,
      [string]$NpcapSdk = '.deps/npcap-sdk')

$ErrorActionPreference = 'Stop'
$buildDirectory = Join-Path $PSScriptRoot 'build'
$executable = Join-Path $buildDirectory 'netsentry.exe'

if ($Clean) {
    foreach ($name in @('netsentry.exe', 'netsentry-main.i', 'netsentry-main.s', 'netsentry-main.o',
                       'netsentry-capture.i', 'netsentry-capture.s', 'netsentry-capture.o',
                       'wpcap.dll', 'Packet.dll')) {
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
    if ((& $compiler.Source '-dumpmachine') -ne 'x86_64-w64-mingw32') {
        throw 'MSYS2 UCRT64 x64 GCC가 필요합니다.'
    }
    $sdkInclude = Join-Path $NpcapSdk 'Include'
    $sdkLibrary = Join-Path $NpcapSdk 'Lib/x64/wpcap.lib'
    if (-not (Test-Path (Join-Path $sdkInclude 'pcap.h')) -or -not (Test-Path $sdkLibrary)) {
        throw 'Npcap SDK가 없습니다. docs/ENVIRONMENT.md의 로컬 SDK 준비 방법을 확인하세요.'
    }
    $extraFlags = @()
    if ($WarningsAsErrors) { $extraFlags += '-Werror' }
    # 한글 TEMP 경로 문제를 피하도록 중간 파일을 실행 파일 옆에 저장한다.
    & $compiler.Source '-std=c11' '-Wall' '-Wextra' '-Wpedantic' '-save-temps=obj' `
        @extraFlags '-I' 'include' '-I' $sdkInclude 'src/main.c' 'src/capture.c' `
        $sdkLibrary '-o' 'build/netsentry.exe'
    if ($LASTEXITCODE -ne 0) {
        throw "GCC 빌드 실패: 종료 코드 $LASTEXITCODE."
    }
    if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
        throw 'GCC가 netsentry.exe를 생성하지 않았습니다.'
    }
    # 설치된 런타임을 로컬 실행에 사용한다. 전역 PATH와 시스템 파일은 수정하지 않는다.
    foreach ($dll in @('wpcap.dll', 'Packet.dll')) {
        $source = Join-Path ([Environment]::SystemDirectory) "Npcap/$dll"
        if (-not (Test-Path -LiteralPath $source)) {
            throw "Npcap 런타임 파일이 없습니다: $source"
        }
        Copy-Item -LiteralPath $source -Destination (Join-Path $buildDirectory $dll) -Force
    }
}
finally {
    Pop-Location
}
Write-Output "빌드 완료: $executable"
