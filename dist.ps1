#Requires -Version 7
<#
.SYNOPSIS
    Builds a versioned production release of Mellangan and packs it into dist/Mellangan-<version>-<platform>.7z
    (Windows) or .tar.gz (Linux, macOS). Runs on PowerShell 7 on all three.

.DESCRIPTION
    1. Bumps VERSION (MAJOR.MINOR.PATCH) and moves the CHANGELOG.md "Unreleased" notes under the new version.
    2. Configures and builds the Release configuration.
    3. Runs --bench-gate on the Release build: 10,000 enemies must hold 55 fps with a frame p99 under 16.7 ms.
    4. Stages the executable and the runtime resources, then archives them (package.ps1).
    If the build or the bench gate fails, VERSION and CHANGELOG.md are restored.

.PARAMETER Bump
    patch (default), minor, major, or none to rebuild the current version without touching the changelog.

.PARAMETER SkipBench
    Packages without running the bench gate (it needs a window and a GPU).

.EXAMPLE
    ./dist.ps1 -Bump minor
#>
[CmdletBinding()]
param(
    [ValidateSet('major', 'minor', 'patch', 'none')]
    [string]$Bump = 'patch',
    [switch]$SkipBench
)

$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

function Invoke-Checked([string]$what, [scriptblock]$cmd) {
    & $cmd
    if ($LASTEXITCODE) { throw "$what failed (exit $LASTEXITCODE)" }
}

# --- version -----------------------------------------------------------------------------------------------------
$versionText = Get-Content VERSION -Raw
$changelogText = Get-Content CHANGELOG.md -Raw
if ($versionText.Trim() -notmatch '^(\d+)\.(\d+)\.(\d+)$') { throw "VERSION must be MAJOR.MINOR.PATCH, got '$($versionText.Trim())'" }
[int]$major, [int]$minor, [int]$patch = $Matches[1..3]
switch ($Bump) {
    'major' { $major++; $minor = 0; $patch = 0 }
    'minor' { $minor++; $patch = 0 }
    'patch' { $patch++ }
}
$version = "$major.$minor.$patch"

if ($Bump -ne 'none') {
    $lines = $changelogText -split "`r?`n"
    $start = [array]::IndexOf($lines, '## [Unreleased]')
    if ($start -lt 0) { throw "CHANGELOG.md has no '## [Unreleased]' heading" }
    $end = $start + 1
    while ($end -lt $lines.Count -and $lines[$end] -notmatch '^## \[') { $end++ }
    $notes = @($lines[($start + 1)..($end - 1)])
    if (-not ($notes | Where-Object { $_ -match '\S' -and $_ -notmatch '^###' })) {
        throw "Nothing under '## [Unreleased]' in CHANGELOG.md. Describe the changes in $version first, " +
              "or run './dist.ps1 -Bump none' to rebuild $($versionText.Trim()) as is."
    }
    $released = @($lines[0..$start]) + '' + "## [$version] - $(Get-Date -Format 'yyyy-MM-dd')" + $notes
    if ($end -lt $lines.Count) { $released += $lines[$end..($lines.Count - 1)] }
    Set-Content CHANGELOG.md ($released -join "`n") -NoNewline
    Set-Content VERSION $version
}
Write-Host "Mellangan $version" -ForegroundColor Cyan

# --- build -------------------------------------------------------------------------------------------------------
try {
    if ($IsWindows -and -not $env:VULKAN_SDK) {
        $sdk = Get-ChildItem 'C:\VulkanSDK' -Directory -ErrorAction SilentlyContinue | Sort-Object { [version]$_.Name } | Select-Object -Last 1
        if (-not $sdk) { throw 'Vulkan SDK not found: set VULKAN_SDK' }
        $env:VULKAN_SDK = $sdk.FullName
    }
    Invoke-Checked 'configure' { cmake -S . -B build -DCMAKE_BUILD_TYPE=Release }
    Invoke-Checked 'build' { cmake --build build --config Release --parallel }
    if (-not $SkipBench) {
        $app = @('build/bin/Release/MellanganApp.exe', 'build/bin/Release/Mellangan', 'build/bin/Mellangan.app/Contents/MacOS/Mellangan',
                 'build/bin/Mellangan') | Where-Object { Test-Path $_ } | Select-Object -First 1
        if (-not $app) { throw 'bench gate: Release executable not found' }
        $run = Start-Process -FilePath $app -ArgumentList '--bench-gate' -Wait -PassThru
        $log = Join-Path (Split-Path $app) 'mellangan.log'
        if (Test-Path $log) { Select-String -Path $log -Pattern 'bench' | ForEach-Object { Write-Host $_.Line } }
        if ($run.ExitCode) { throw "bench gate failed (exit $($run.ExitCode)); see $log, or pass -SkipBench" }
    }
} catch {
    if ($Bump -ne 'none') {
        Set-Content VERSION $versionText -NoNewline
        Set-Content CHANGELOG.md $changelogText -NoNewline
        Write-Warning 'Build or bench gate failed; VERSION and CHANGELOG.md restored.'
    }
    throw
}

# --- package -----------------------------------------------------------------------------------------------------
& (Join-Path $PSScriptRoot package.ps1) -Version $version | Out-Null
