#Requires -Version 7
<#
.SYNOPSIS
    Builds a versioned production release of Mellangan and packs it into dist/Mellangan-<version>.7z.

.DESCRIPTION
    1. Bumps VERSION (MAJOR.MINOR.PATCH) and moves the CHANGELOG.md "Unreleased" notes under the new version.
    2. Configures and builds the Release configuration.
    3. Stages the executable and the runtime resources, then 7-Zips them.
    If the build fails, VERSION and CHANGELOG.md are restored.

.PARAMETER Bump
    patch (default), minor, major, or none to rebuild the current version without touching the changelog.

.EXAMPLE
    ./dist.ps1 -Bump minor
#>
[CmdletBinding()]
param(
    [ValidateSet('major', 'minor', 'patch', 'none')]
    [string]$Bump = 'patch'
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
    if (-not $env:VULKAN_SDK) {
        $sdk = Get-ChildItem 'C:\VulkanSDK' -Directory -ErrorAction SilentlyContinue | Sort-Object { [version]$_.Name } | Select-Object -Last 1
        if (-not $sdk) { throw 'Vulkan SDK not found: set VULKAN_SDK' }
        $env:VULKAN_SDK = $sdk.FullName
    }
    Invoke-Checked 'configure' { cmake -S . -B build }
    Invoke-Checked 'build' { cmake --build build --config Release --parallel }
} catch {
    if ($Bump -ne 'none') {
        Set-Content VERSION $versionText -NoNewline
        Set-Content CHANGELOG.md $changelogText -NoNewline
        Write-Warning 'Build failed; VERSION and CHANGELOG.md restored.'
    }
    throw
}

# --- package -----------------------------------------------------------------------------------------------------
$name = "Mellangan-$version"
$stage = Join-Path dist $name
$archive = Join-Path dist "$name.7z"
Remove-Item $stage, $archive -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory "$stage/res/shaders" -Force | Out-Null

$bin = 'build/Release'
Copy-Item "$bin/Mellangan.exe", CHANGELOG.md $stage
Copy-Item res/atlas.png, res/particles.png, res/font.png, res/CREDITS.md "$stage/res"
Copy-Item "$bin/res/shaders/*.spv" "$stage/res/shaders"
Copy-Item res/sfx, res/music "$stage/res" -Recurse

$7z = (Get-Command 7z -ErrorAction SilentlyContinue).Source
if (-not $7z) { $7z = "$env:ProgramFiles\7-Zip\7z.exe" }
if (-not (Test-Path $7z)) { throw '7-Zip not found: install it or put 7z on PATH' }
Push-Location dist
try { Invoke-Checked '7z' { & $7z a -t7z -mx=9 "$name.7z" $name | Out-Null } } finally { Pop-Location }

$size = (Get-Item $archive).Length / 1MB
Write-Host ("{0} ({1:N1} MB)" -f (Resolve-Path $archive), $size) -ForegroundColor Green
