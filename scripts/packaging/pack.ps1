# Packages plugin files for publishing to GitHub Releases

function packFiles()
{
    Remove-Item "package-release" -Force -Recurse -ErrorAction SilentlyContinue
    New-Item "package-release" -ItemType Directory

    $exclude = @(
        'Sentry.uplugin',
        '.gitkeep',
        '.DS_Store',
        'Binaries',
        'Intermediate'
    )

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

        $newPluginSpec = $pluginSpec

        # Adding EngineVersion key may cause warnings when using package in licensee engine builds (https://github.com/getsentry/sentry-unreal/issues/811)

        $newPluginSpec = @($pluginSpec[0..0]) + @('	"EngineVersion" : "' + $engineVersion + '.0",') + @($pluginSpec[1..($pluginSpec.count)])

        # Handle platform name difference for UE 4.27
        if ($engineVersion -eq "4.27")
        {
            $newPluginSpec = $newPluginSpec -replace '"LinuxArm64"', '"LinuxAArch64"'
        }

        $newPluginSpec | Out-File "package-release/Sentry.uplugin"

        Remove-Item -ErrorAction SilentlyContinue $packageName

        # Workaround for Compress-Archive discarding file permissions
        # ======
        # Use of [System.IO.Compression.ZipFile]::CreateFromDirectory instead of Compress-Archive (or a third-party tool)
        # so that we retain file permissions
        # For more information, see https://github.com/PowerShell/Microsoft.PowerShell.Archive/issues/36
        # NOTE: This requires .NET 6+: https://github.com/dotnet/runtime/issues/1548
        Push-Location package-release
        try
        {
            $location = Get-Location
            # Workaround in order to support LTS PowerShell version 5.1 on Windows
            if ([Environment]::GetEnvironmentVariable("OS") -Match "Windows")
            {
                Add-Type -AssemblyName "System.IO.Compression.FileSystem"
            }
            [System.IO.Compression.ZipFile]::CreateFromDirectory($location, "$location/../$packageName")
        }
        finally
        {
            Pop-Location
        }
    }
}

packFiles
