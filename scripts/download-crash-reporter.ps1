Set-StrictMode -Version latest

$conf = Get-Content "$PSScriptRoot/../modules/sentry-crash-reporter.properties" -Raw | ConvertFrom-StringData
$baseUrl = "$($conf.repo)/releases/download/$($conf.version)/sentry-desktop-crash-reporter-$($conf.version)"
$targetDir = "$PSScriptRoot/../plugin-dev/Source/ThirdParty"

$platforms = @{
    "Win64"     = "win-x64"
    "WinArm64"  = "win-arm64"
    "Linux"     = "linux-x64"
    "LinuxArm64"= "linux-arm64"
    "Mac"       = "osx-arm64"
}

foreach ($platform in $platforms.Keys)
{
    $runtimeId = $platforms[$platform]

    if ($runtimeId.StartsWith("win-"))
    {
        $archiveUrl = "$baseUrl-$runtimeId.zip"
        $executableName = "Sentry.CrashReporter.exe"
    }
    else
    {
        $archiveUrl = "$baseUrl-$runtimeId.tar.gz"
        $executableName = "Sentry.CrashReporter"
    }

    Write-Host "Downloading Crash Reporter for $platform ($runtimeId) ..."

    $tempDir = Join-Path ([System.IO.Path]::GetTempPath()) "sentry-crash-reporter-download-$platform"
    if (Test-Path $tempDir)
    {
        Remove-Item $tempDir -Recurse -Force
    }
    New-Item $tempDir -ItemType Directory -Force > $null

    $destDir = "$targetDir/$platform"
    if (-not (Test-Path $destDir))
    {
        New-Item $destDir -ItemType Directory -Force > $null
    }

    if ($runtimeId.StartsWith("win-"))
    {
        $archivePath = "$tempDir/archive.zip"
        Invoke-WebRequest -Uri $archiveUrl -OutFile $archivePath
        Expand-Archive -Path $archivePath -DestinationPath $tempDir -Force
    }
    else
    {
        $archivePath = "$tempDir/archive.tar.gz"
        Invoke-WebRequest -Uri $archiveUrl -OutFile $archivePath
        tar -xzf $archivePath -C $tempDir "./$executableName"
    }

    Copy-Item "$tempDir/$executableName" -Destination "$destDir/$executableName"

    if (Get-Command 'chmod' -ErrorAction SilentlyContinue)
    {
        chmod +x "$destDir/$executableName"
    }

    Remove-Item $tempDir -Recurse -Force
}
