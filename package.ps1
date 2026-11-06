#Requires -Version 7
<#
.SYNOPSIS
    Packs an existing Release build into dist/Mellangan-<version>-<platform>.7z (Windows) or .tar.gz (Linux, macOS).

.DESCRIPTION
    Stages the executable, CHANGELOG.md and the runtime resources, then archives them. It does not build:
    dist.ps1 calls it after its build and bench gate, and CI calls it after its Release build.
    Prints the archive path as its last line of output.

.PARAMETER Version
    The version in the file name. Defaults to the VERSION file.

.EXAMPLE
    ./package.ps1
#>
[CmdletBinding()]
param([string]$Version = (Get-Content (Join-Path $PSScriptRoot VERSION) -Raw).Trim())

$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

$platform = if ($IsWindows) { 'windows' } elseif ($IsMacOS) { 'macos' } else { 'linux' }
$name = "Mellangan-$Version-$platform"
$stage = Join-Path dist $name
$file = if ($IsWindows) { "$name.7z" } else { "$name.tar.gz" }
$archive = Join-Path dist $file
Remove-Item $stage, $archive -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory "$stage/res/shaders" -Force | Out-Null

$exe = if ($IsWindows) { if (Test-Path "build/bin/Release/MellanganApp.exe") { 'MellanganApp.exe' } else { 'Mellangan.exe' } } else { 'Mellangan' }
$bin = if (Test-Path "build/bin/Release/$exe") { 'build/bin/Release' } elseif (Test-Path "build/app/Release/$exe") { 'build/app/Release' } elseif (Test-Path "build/Release/$exe") { 'build/Release' } else { 'build' }
if (-not (Test-Path "$bin/$exe")) { throw "Release executable not found: build it first" }
$targetName = if ($IsWindows) { 'Mellangan.exe' } else { 'Mellangan' }
Copy-Item "$bin/$exe" (Join-Path $stage $targetName)
Copy-Item CHANGELOG.md $stage
Copy-Item res/atlas.png, res/particles.png, res/weather.png, res/font.png, res/balance.ini, res/CREDITS.md "$stage/res"
Copy-Item "$bin/res/shaders/*.spv" "$stage/res/shaders"
Copy-Item res/sfx, res/music, res/videos "$stage/res" -Recurse

Push-Location dist
try {
    if ($IsWindows) {
        $7z = (Get-Command 7z -ErrorAction SilentlyContinue).Source
        if (-not $7z) { $7z = "$env:ProgramFiles\7-Zip\7z.exe" }
        if (-not (Test-Path $7z)) { throw '7-Zip not found: install it or put 7z on PATH' }
        & $7z a -t7z -mx=9 $file $name | Out-Null
    } else {
        tar -czf $file $name
    }
    if ($LASTEXITCODE) { throw "archive failed (exit $LASTEXITCODE)" }
} finally { Pop-Location }

$size = (Get-Item $archive).Length / 1MB
Write-Host ("{0} ({1:N1} MB)" -f (Resolve-Path $archive), $size) -ForegroundColor Green
(Resolve-Path $archive).Path
