Set-StrictMode -Version latest

$conf = Get-Content "$PSScriptRoot/../plugin-dev/sentry-cli.properties" -Raw | ConvertFrom-StringData
$platforms = @('Darwin-universal', 'Linux-x86_64', 'Windows-x86_64')
$targetDir = "$PSScriptRoot/../plugin-dev/Source/ThirdParty/CLI"
$baseUrl = "$($conf.repo)/releases/download/$($conf.version)/sentry-cli-"

if (-not (Test-Path $targetDir))
{
    New-Item $targetDir -ItemType Directory > $null
}

# Remove previous contents of the directory (remove all ignored files).
git clean -fXd $targetDir

foreach ($name in $platforms)
{
    if ($name.StartsWith('Windows'))
    {
        $name += '.exe';
    }

    $targetFile = "$targetDir/sentry-cli-$name"
    Write-Host "Downloading $targetFile"
    Invoke-WebRequest -Uri "$baseUrl$name" -OutFile $targetFile

    if (Get-Command 'chmod' -ErrorAction SilentlyContinue)
    {
        chmod +x $targetFile
    }
}