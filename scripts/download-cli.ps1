Set-StrictMode -Version latest

$conf = Get-Content "$PSScriptRoot/../plugin-dev/sentry-cli.properties" -Raw | ConvertFrom-StringData
$platforms = @('darwin-arm64', 'linux-x64', 'windows-x64.exe')
$targetDir = "$PSScriptRoot/../plugin-dev/Source/ThirdParty/CLI"
$baseUrl = "$($conf.repo)/releases/download/cli@$($conf.version)/sentry-"

if (-not (Test-Path $targetDir))
{
    New-Item $targetDir -ItemType Directory > $null
}

# Remove previous contents of the directory (remove all ignored files).
git clean -fXd $targetDir

foreach ($name in $platforms)
{
    $targetFile = "$targetDir/sentry-$name"
    Write-Host "Downloading $targetFile"
    Invoke-WebRequest -Uri "$baseUrl$name" -OutFile $targetFile

    if (Get-Command 'chmod' -ErrorAction SilentlyContinue)
    {
        chmod +x $targetFile
    }
}
