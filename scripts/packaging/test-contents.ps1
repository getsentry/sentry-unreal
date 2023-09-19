# Verifies the contents of the plugin package against a snapshot files
# for two publishing platforms - GitHub and UE Marketplace.
# 'pack.ps1' must have already been run.

# To accept a new snapshot file, run 'pwsh ./test-contents.ps1 accept'

$ErrorActionPreference = "Stop"

$projectRoot = "$PSScriptRoot/../.."

function testFiles([string] $publishingPlatform)
{
    $snapshotFile = "$PSScriptRoot/package-$publishingPlatform.snapshot"

    if (-not(Test-Path -Path $snapshotFile))
    {
        Write-Host "Snapshot file '$snapshotFile' not found.
    Can't compare package contents against baseline."
        exit 2
    }

    $packages = Get-ChildItem "$projectRoot/sentry-unreal-*-$publishingPlatform.zip"
    $expectedPackagesCount = (Get-Content "$PSScriptRoot/engine-versions.txt").Length
    if ($packages.Length -ne $expectedPackagesCount)
    {
        throw "Invalid number of packages - expected $expectedPackagesCount, got $packages"
    }

    foreach ($packageFile in $packages)
    {
        Write-Host "Testing $packageFile contents"
        $zip = [IO.Compression.ZipFile]::OpenRead($packageFile)
        try
        {
            $snapshotContent = $zip.Entries.FullName.Replace("\", "/") | Sort-Object
            if ($args.Count -gt 0 -and $args[0] -eq "accept")
            {
                # Override the snapshot file with the current package contents
                $snapshotContent | Out-File $snapshotFile
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

# TODO
# $androidLibsDir = "$projectRoot/modules/sentry-java/sentry-android-ndk/build/intermediates/merged_native_libs/release/out/lib/"
# if (-not(Test-Path -Path $androidLibsDir)) {
#     Write-Host  "Android native libs not found in: '$androidLibsDir'"
#     exit 1
# }

# $androidLibs = Get-ChildItem -Recurse $androidLibsDir | ForEach-Object {$_.Directory.Name + "/" + $_.Name}
# $result = Compare-Object $androidLibs (Get-Content "$PSScriptRoot/android-libs.snapshot")
# if ($result.count -eq 0) {
#     Write-Host  "Android native libs match snapshot."
# }
# else {
#     Write-Host  "Android native libs do not match snapshot."
#     $result | Format-Table -AutoSize
#     exit 3
# }