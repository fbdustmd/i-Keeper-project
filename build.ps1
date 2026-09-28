[CmdletBinding()]
param([switch]$Clean)

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
    # Keep intermediate files beside the output, avoiding the non-ASCII TEMP path.
    & $compiler.Source '-std=c11' '-Wall' '-Wextra' '-Wpedantic' '-save-temps=obj' `
        'src/main.c' '-o' 'build/netsentry.exe'
    if ($LASTEXITCODE -ne 0) {
        throw "GCC failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}
Write-Output "Built: $executable"
