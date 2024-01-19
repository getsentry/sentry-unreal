# Packages plugin files for two publishing platforms - GitHub and UE Marketplace.

# The UE Marketplace version doesn't include `.exe`, `.bat` and `.sh` files due to certain
# platform restrictions: https://www.unrealengine.com/en-US/marketplace-guidelines#262e
# Plugin provides tools allowing to download these files manually.

function packFiles([string] $publishingPlatform)
{
    Remove-Item "package-release-$publishingPlatform" -Force -Recurse -ErrorAction SilentlyContinue
    New-Item "package-release-$publishingPlatform" -ItemType Directory

    $exclude = @(
        'Sentry.uplugin',
        '.gitkeep',
        '.DS_Store',
        'Binaries',
        'Intermediate'
    )

    if ($publishingPlatform -eq "marketplace")
    {
        $exclude += @(
            'sentry-cli-Windows-x86_64.exe',
            'upload-debug-symbols-win.bat',
            'upload-debug-symbols.sh',
            'crashpad_handler.exe'
        )
    }

    $sentryModuleScriptPath = "plugin-dev/Source/Sentry/Private/SentryModule.cpp"
    $sentryModuleScript = Get-Content $sentryModuleScriptPath
    if ($publishingPlatform -eq "marketplace")
    {
        $sentryModuleScript -replace 'FSentryModule::IsMarketplace = false', 'FSentryModule::IsMarketplace = true' | Out-File $sentryModuleScriptPath
    }

    Copy-Item "plugin-dev/*" "package-release-$publishingPlatform/" -Exclude $exclude -Recurse
    Copy-Item "CHANGELOG.md" -Destination "package-release-$publishingPlatform/CHANGELOG.md"
    Copy-Item "LICENSE" -Destination "package-release-$publishingPlatform/LICENSE"

    $pluginSpec = Get-Content "plugin-dev/Sentry.uplugin"
    $version = [regex]::Match("$pluginSpec", '"VersionName": "([^"]+)"').Groups[1].Value
    $engineVersions = Get-Content $PSScriptRoot/engine-versions.txt
    foreach ($engineVersion in $engineVersions)
    {
        $packageName = "sentry-unreal-$version-engine$engineVersion-$publishingPlatform.zip"
        Write-Host "Creating a release package for Unreal $engineVersion as $packageName"

        $newPluginSpec = @($pluginSpec[0..0]) + @('	"EngineVersion" : "' + $engineVersion + '.0",') + @($pluginSpec[1..($pluginSpec.count)])
        $newPluginSpec | Out-File "package-release-$publishingPlatform/Sentry.uplugin"

        Remove-Item -ErrorAction SilentlyContinue $packageName

        # Note: unlike `zip` (the info-ZIP program), Compress-archive doesn't preserve file permissions - messing up later usage on unix-based systems.
        Push-Location package-release-$publishingPlatform
        try
        {
            zip -r -1 -v ../$packageName ./*
        }
        finally
        {
            Pop-Location
        }
    }
}

packFiles("github")
packFiles("marketplace")