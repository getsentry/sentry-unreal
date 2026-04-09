Set-StrictMode -Version latest

Write-Host "Downloading native SDKs from the latest CI pipeline"

function findCiRun([string] $branch)
{
    Write-Host "Looking for the latest successful CI run on branch '$branch'"
    $jsonArray = gh run list --branch $branch --workflow package-plugin-workflow --json 'conclusion,databaseId' | ConvertFrom-Json
    $id = $jsonArray | Where-Object 'conclusion' -EQ 'success' | Select-Object -First 1 -ExpandProperty 'databaseId'
    if ( "$id" -eq "" )
    {
        Write-Warning "  ... no successful CI run found on $branch"
    }
    else
    {
        Write-Host "  ... found CI run ID: $id"
        "$id"
    }
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

Write-Host "Downloading Crash Reporter binaries ..."
gh run download $runId -n "CrashReporter-Win64" -D "$outDir/Win64"
gh run download $runId -n "CrashReporter-WinArm64" -D "$outDir/WinArm64"
gh run download $runId -n "CrashReporter-Linux" -D "$outDir/Linux"
gh run download $runId -n "CrashReporter-LinuxArm64" -D "$outDir/LinuxArm64"
gh run download $runId -n "CrashReporter-Mac" -D "$outDir/Mac"
