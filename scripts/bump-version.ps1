Param(
    [Parameter(Mandatory = $true)][String]$PrevVersion,
    [Parameter(Mandatory = $true)][String]$NewVersion
)
Set-StrictMode -Version latest

$pluginFile = "$PSScriptRoot/../plugin-dev/Sentry.uplugin"

$content = Get-Content $pluginFile

$content -replace '"VersionName": ".*"', ('"VersionName": "' + $NewVersion + '"') | Out-File $pluginFile
if ("$content" -eq "$(Get-Content $pluginFile)")
{
    $versionInFile = [regex]::Match("$content", '"VersionName": "([^"]+)"').Groups[1].Value
    if ("$versionInFile" -ne "$NewVersion")
    {
        Throw "Failed to update version in $pluginFile - the content didn't change. The version found in the file is '$versionInFile'."
    }
}
