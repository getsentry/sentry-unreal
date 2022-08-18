Set-StrictMode -Version latest

if (-not $IsWindows)
{
    throw "This script is only supposed to be run on Windows. Use init.sh on macOS/Linux instead."
}

$repoRoot = Resolve-Path "$PSScriptRoot/.."

function setupPluginLink([string] $projectDir)
{
    $targetDir = Get-Item "$repoRoot/plugin-dev"
    $linkPath = "$repoRoot/$projectDir/Plugins/sentry"

    if (Test-Path $linkPath)
    {
        $link = Get-Item $linkPath
        if (($link.LinkType -ne "Junction") -or ("$($link.Target)" -ne "$targetDir"))
        {
            Write-Warning "Removing existing link $linkPath because it doesn't match the expected target or type"
            $link.Delete()
        }
        else
        {
            Write-Host "Link $linkPath already exists and matches target $targetDir - skipping"
            return
        }
    }

    $parentDir = Split-Path -Parent $linkPath
    If (-not (Test-Path $parentDir))
    {
        New-Item -ItemType Directory -Path $parentDir
    }

    Write-Host "Creating a link (directory junction) from $linkPath to $targetDir"
    New-Item $linkPath -ItemType Junction -Target $targetDir > $null
}

setupPluginLink "sample"
. "$PSScriptRoot/download-sdks.ps1"