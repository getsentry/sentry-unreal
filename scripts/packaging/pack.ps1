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
            'crashpad_wer.dll'
        )
    }

    Copy-Item "plugin-dev/*" "package-release-$publishingPlatform/" -Exclude $exclude -Recurse
    Copy-Item "CHANGELOG.md" -Destination "package-release-$publishingPlatform/CHANGELOG.md"
    Copy-Item "LICENSE" -Destination "package-release-$publishingPlatform/LICENSE"

    if ($publishingPlatform -eq "marketplace")
    {
        # Workaround for PowerShell 5.1 writing UTF8-BOM
        # ======
        # Set current directory so that ::WriteAllLines can accept a relative path
        [System.Environment]::CurrentDirectory = (Get-Location).Path

        # Find the specific package version of the SentryModule.cpp file
        $sentryModuleCppPath = "package-release-$publishingPlatform/Source/Sentry/Private/SentryModule.cpp"

        # We know the file is meant to be UTF8, so let's be explicit
        $sentryModuleCppContents = Get-Content $sentryModuleCppPath -Encoding UTF8
        $replacedCppContents = $sentryModuleCppContents -replace 'FSentryModule::IsMarketplace = false', 'FSentryModule::IsMarketplace = true'

        # PowerShell 5.1 will write UT8-BOM if we use Out-File, so bypass this issue and use ::WriteAllLines
        [System.IO.File]::WriteAllLines($sentryModuleCppPath, $replacedCppContents)
    }

    $pluginSpec = Get-Content "plugin-dev/Sentry.uplugin"
    $version = [regex]::Match("$pluginSpec", '"VersionName": "([^"]+)"').Groups[1].Value
    $engineVersions = Get-Content $PSScriptRoot/engine-versions.txt
    foreach ($engineVersion in $engineVersions)
    {
        $packageName = "sentry-unreal-$version-engine$engineVersion-$publishingPlatform.zip"
        Write-Host "Creating a release package for Unreal $engineVersion as $packageName"

        $newPluginSpec = $pluginSpec

        # Add EngineVersion key only for marketplace package to avoid warnings in licensee versions of Unreal
        # where github package is used (https://github.com/getsentry/sentry-unreal/issues/811)
        if ($publishingPlatform -eq "marketplace")
        {
            $newPluginSpec = @($pluginSpec[0..0]) + @('	"EngineVersion" : "' + $engineVersion + '.0",') + @($pluginSpec[1..($pluginSpec.count)])
        }

        # Handle platform name difference for UE 4.27
        if ($engineVersion -eq "4.27")
        {
            $newPluginSpec = $newPluginSpec -replace '"LinuxArm64"', '"LinuxAArch64"'
        }

        $newPluginSpec | Out-File "package-release-$publishingPlatform/Sentry.uplugin"

        Remove-Item -ErrorAction SilentlyContinue $packageName

        # Workaround for Compress-Archive discarding file permissions
        # ======
        # Use of [System.IO.Compression.ZipFile]::CreateFromDirectory instead of Compress-Archive (or a third-party tool)
        # so that we retain file permissions
        # For more information, see https://github.com/PowerShell/Microsoft.PowerShell.Archive/issues/36
        # NOTE: This requires .NET 6+: https://github.com/dotnet/runtime/issues/1548
        Push-Location package-release-$publishingPlatform
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

packFiles("github")
packFiles("marketplace")
