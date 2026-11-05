#Requires -Version 7
<#
.SYNOPSIS
    Downloads the Windows package CI built for a commit into dist/, for attaching to a GitHub release.

.DESCRIPTION
    Finds the CI run of the commit's push to main, checks it finished green, and downloads its
    Mellangan-windows artifact: dist/Mellangan-<version>-windows.7z, with the version read from VERSION at that
    commit. CI builds from a clean checkout with /W4 /WX after --test passes, but it has no GPU, so the bench gate
    still runs locally through dist.ps1. Prints the archive path as its last line of output. Needs gh.

.PARAMETER Ref
    The commit, tag or branch to fetch. Defaults to HEAD.

.EXAMPLE
    ./ci-artifact.ps1 v0.6.5
#>
[CmdletBinding()]
param([string]$Ref = 'HEAD')

$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

$sha = (git rev-parse "$Ref^{commit}").Trim()
if ($LASTEXITCODE) { throw "unknown ref '$Ref'" }
$version = (git show "${sha}:VERSION").Trim()
$runs = gh run list --workflow ci.yml --commit $sha --event push --limit 1 --json databaseId,status,conclusion,url | ConvertFrom-Json
if (-not $runs) { throw "no CI run for $sha yet: push it to main first" }
$run = $runs[0]
if ($run.status -ne 'completed') { throw "CI for $sha is still $($run.status): gh run watch $($run.databaseId)" }
if ($run.conclusion -ne 'success') { throw "CI for $sha ended in $($run.conclusion): $($run.url)" }

$file = "Mellangan-$version-windows.7z"
$archive = Join-Path dist $file
New-Item -ItemType Directory dist -Force | Out-Null
Remove-Item $archive -Force -ErrorAction SilentlyContinue
gh run download $run.databaseId --name Mellangan-windows --dir dist
if ($LASTEXITCODE) { throw "download failed: the run may predate CI packaging or its artifact may have expired" }
if (-not (Test-Path $archive)) { throw "the artifact does not hold $file" }

$size = (Get-Item $archive).Length / 1MB
Write-Host ("{0} ({1:N1} MB) from {2}" -f (Resolve-Path $archive), $size, $run.url) -ForegroundColor Green
(Resolve-Path $archive).Path
