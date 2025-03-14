# Verifies the contents of the plugin package against a snapshot files
# for two publishing platforms - GitHub and UE Marketplace.
# 'pack.ps1' must have already been run.

# To accept a new snapshot file, run 'pwsh ./test-contents.ps1 accept'

$ErrorActionPreference = "Stop"

$projectRoot = "$PSScriptRoot/../.."

$accept = $args.Count -gt 0 -and $args[0] -eq "accept"

function testFiles([string] $publishingPlatform)
{
    $snapshotFile = "$PSScriptRoot/package-$publishingPlatform.snapshot"

    if (-not(Test-Path -Path $snapshotFile))
    {
        Write-Host "Snapshot file '$snapshotFile' not found.
    Can't compare package contents against baseline."
        exit 2
    }

    # TODO: Extract this functionality to share between `test-contents.ps1` and `pack.ps1`
    # See https://github.com/getsentry/sentry-unreal/issues/820 for more details.
    $pluginSpec = Get-Content "plugin-dev/Sentry.uplugin"
    $version = [regex]::Match("$pluginSpec", '"VersionName": "([^"]+)"').Groups[1].Value

    Write-Host "Searching version $version release packages..."

    $packages = Get-ChildItem -Path "$projectRoot/*" -Include "sentry-unreal-$version-engine*-$publishingPlatform.zip"
    $expectedPackagesCount = (Get-Content "$PSScriptRoot/engine-versions.txt").Length
    if ($packages.Length -ne $expectedPackagesCount)
    {
        throw "Invalid number of packages - expected $expectedPackagesCount, got $($packages.Length)"
    }

    foreach ($packageFile in $packages)
    {
        Write-Host "Testing $packageFile contents"
        $zip = [IO.Compression.ZipFile]::OpenRead($packageFile)
        try
        {
            $snapshotContent = $zip.Entries.FullName.Replace("\", "/") | Sort-Object
            if ($accept)
            {
                # Override the snapshot file with the current package contents
                [System.IO.File]::WriteAllLines($snapshotFile, $snapshotContent)
            }
            $result = Compare-Object $snapshotContent (Get-Content $snapshotFile)
            if ($result.count -eq 0)
            {
                Write-Host "  PASS - package content matches the snapshot."
            }
            else
            {
                Write-Host "  FAIL - package content does not match the snapshot."
                $result | Format-Table -AutoSize
                exit 3
            }
        }
        finally
        {
            $zip.Dispose()
        }
    }
}

testFiles("github")
testFiles("marketplace")