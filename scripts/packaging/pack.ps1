Remove-Item "package-release" -Recurse -ErrorAction SilentlyContinue
New-Item "package-release" -ItemType Directory

$exclude = @('Sentry.uplugin')

Copy-Item "plugin-dev/*" "package-release/" -Exclude $exclude -Recurse
Copy-Item "CHANGELOG.md" -Destination "package-release/CHANGELOG.md"
Copy-Item "LICENSE" -Destination "package-release/LICENSE"

$pluginSpec = Get-Content "plugin-dev/Sentry.uplugin"
$version = [regex]::Match("$pluginSpec", '"VersionName": "([^"]+)"').Groups[1].Value
$engineVersions = Get-Content $PSScriptRoot/engine-versions.txt
foreach ($engineVersion in $engineVersions)
{
    $packageName = "sentry-unreal-$version-engine$engineVersion.zip"
    Write-Host "Creating a release package for Unreal $engineVersion as $packageName"

    $newPluginSpec = @($pluginSpec[0..0]) + @('	"EngineVersion" : "' + $engineVersion + '",') + @($pluginSpec[1..($pluginSpec.count)])
    $newPluginSpec | Out-File 'package-release/Sentry.uplugin'

    # Note: unlike `zip` (the info-ZIP program), Compress-archive doesn't preserve file permissions.
    # If we want to do that, we should switch to `zip` call and only package on Linux/macOS.
    Compress-Archive "package-release/*" -DestinationPath $packageName -Force
}