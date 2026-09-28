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
    # Keep intermediate files beside the output, avoiding the non-ASCII TEMP path.
    & $compiler.Source '-std=c11' '-Wall' '-Wextra' '-Wpedantic' '-save-temps=obj' `
        @extraFlags 'src/main.c' '-o' 'build/netsentry.exe'
    if ($LASTEXITCODE -ne 0) {
        throw "GCC failed with exit code $LASTEXITCODE."
    }
    if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
        throw 'GCC did not produce netsentry.exe.'
    }
}
finally {
    Pop-Location
}
Write-Output "Built: $executable"
