Set-StrictMode -Version latest

Write-Host "Downloading native SDKs from the latest CI pipeline"

function findCiRun([string] $branch)
{
    Write-Host "Looking for the latest successful CI run on branch '$branch'"
    $encodedBranch = [uri]::EscapeDataString($branch)
    $commits = @()
    if ($branch -ne "HEAD")
    {
        $commits = @(gh api "repos/{owner}/{repo}/commits?sha=$encodedBranch&per_page=20" --jq '.[].sha' 2>$null)
        if ($LASTEXITCODE -ne 0)
        {
            $commits = @()
        }
    }
    foreach ($sha in $commits)
    {
        $id = gh api "repos/{owner}/{repo}/actions/workflows/ci.yml/runs?head_sha=$sha&status=success&per_page=1" --jq '.workflow_runs[0].id // empty'
        if ( "$id" -ne "" )
        {
            Write-Host "  ... found CI run ID: $id (commit $sha)"
            return "$id"
        }
    }
    Write-Warning "  ... no successful CI run found on $branch"
}

$runId = findCiRun("$(git rev-parse --abbrev-ref HEAD)")
if ( "$runId" -eq "" )
{
    $runId = findCiRun("main")
    if ( "$runId" -eq "" )
    {
        exit 1
    }
}

$outDir = "$(Resolve-Path "$PSScriptRoot/../plugin-dev/Source")/ThirdParty"
if (-not (Test-Path $outDir))
{
    New-Item $outDir -ItemType Directory > $null
}

# Mobile platforms: single artifact per platform
$otherSdks = @("Android", "IOS")
foreach ($sdk in $otherSdks)
{
    $sdkDir = "$outDir/$sdk"

    Write-Host "Downloading $sdk SDK to $sdkDir ..."
    if (Test-Path $sdkDir)
    {
        Remove-Item "$sdkDir" -Recurse
    }

    gh run download $runId -n "$sdk-sdk" -D $sdkDir
}

# Mac: cocoa SDK goes into Mac/Cocoa, native SDK into Mac/Native
Write-Host "Downloading Mac Cocoa SDK to $outDir/Mac/Cocoa ..."
if (Test-Path "$outDir/Mac/Cocoa")
{
    Remove-Item "$outDir/Mac/Cocoa" -Recurse
}
gh run download $runId -n "Mac-cocoa-sdk" -D "$outDir/Mac"

Write-Host "Downloading Mac Native SDK to $outDir/Mac/Native ..."
if (Test-Path "$outDir/Mac/Native")
{
    Remove-Item "$outDir/Mac/Native" -Recurse
}
gh run download $runId -n "Mac-native-sdk" -D "$outDir/Mac/Native"

# Native platforms: two backend variants per platform
$nativePlatforms = @("Linux", "LinuxArm64", "Win64", "WinArm64")
foreach ($platform in $nativePlatforms)
{
    foreach ($backend in @("crashpad", "native"))
    {
        $backendDir = if ($backend -eq "crashpad") { "Crashpad" } else { "Native" }
        $targetDir = "$outDir/$platform/$backendDir"

        Write-Host "Downloading $platform-$backend SDK to $targetDir ..."
        if (Test-Path $targetDir)
        {
            Remove-Item "$targetDir" -Recurse
        }

        gh run download $runId -n "$platform-$backend-sdk" -D $targetDir
    }
}

