# Packages plugin files for publishing to GitHub Releases

. $PSScriptRoot/pack-utils.ps1

$projectRoot = "$PSScriptRoot/../.."

function packFiles()
{
    Remove-Item "$projectRoot/package-release" -Force -Recurse -ErrorAction SilentlyContinue
    New-Item "$projectRoot/package-release" -ItemType Directory

    $exclude = @(
        'Sentry.uplugin',
        '.gitkeep',
        '.DS_Store',
        'Binaries',
        'Intermediate'
    )

    Copy-Item "$projectRoot/plugin-dev/*" "$projectRoot/package-release/" -Exclude $exclude -Recurse
    Copy-Item "$projectRoot/CHANGELOG.md" -Destination "$projectRoot/package-release/CHANGELOG.md"
    Copy-Item "$projectRoot/LICENSE" -Destination "$projectRoot/package-release/LICENSE"

    # We know the file is meant to be UTF8, so let's be explicit
    $sentrySubsystemHeader = Get-Content "$projectRoot/plugin-dev/Source/Sentry/Public/SentrySubsystem.h" -Encoding UTF8

    $pluginSpec = Get-Content (Get-PluginSpecPath)
    $pluginVersion = Get-PluginVersion

    $engineVersions = Get-Content "$PSScriptRoot/engine-versions.txt"
    $engineVersions += "generic"

    foreach ($engineVersion in $engineVersions)
    {
        # Adding the EngineVersion key to Sentry.uplugin may trigger warnings in licensee engine builds (https://github.com/getsentry/sentry-unreal/issues/811)
        # Therefore, we include this key only in engine-specific packages.
        # The generic package omits it so that users can download a ready-to-use artifact without having to remove the key manually for that use-case.

        $newPluginSpec = $pluginSpec

        if ($engineVersion -ne "generic")
        {
            $packageName = "sentry-unreal-$pluginVersion-engine$engineVersion.zip"
            Write-Host "Creating a release package for Unreal $engineVersion as $packageName"

            $newPluginSpec = @($pluginSpec[0..0]) + @('	"EngineVersion" : "' + $engineVersion + '.0",') + @($pluginSpec[1..($pluginSpec.count)])
        }
        else
        {
            $packageName = "sentry-unreal-$pluginVersion.zip"
            Write-Host "Creating a generic release package as $packageName"
        }

        # Handle platform name difference for UE 4.27
        if ($engineVersion -eq "4.27")
        {
            $newPluginSpec = $newPluginSpec -replace '"LinuxArm64"', '"LinuxAArch64"'
        }

        $newPluginSpec | Out-File "$projectRoot/package-release/Sentry.uplugin"

        # Replacing raw pointers in UPROPERTY fields with TObjectPtr for UE 5.0+
        # See https://github.com/getsentry/sentry-unreal/issues/1082

        $newSentrySubsystemHeader = $sentrySubsystemHeader

        if ($engineVersion -ne "4.27")
        {
            $newSentrySubsystemHeader = $newSentrySubsystemHeader `
                -replace 'USentryBeforeSendHandler\* BeforeSendHandler;', 'TObjectPtr<USentryBeforeSendHandler> BeforeSendHandler;' `
                -replace 'USentryBeforeBreadcrumbHandler\* BeforeBreadcrumbHandler;', 'TObjectPtr<USentryBeforeBreadcrumbHandler> BeforeBreadcrumbHandler;' `
                -replace 'USentryBeforeLogHandler\* BeforeLogHandler;', 'TObjectPtr<USentryBeforeLogHandler> BeforeLogHandler;' `
                -replace 'USentryTraceSampler\* TraceSampler;', 'TObjectPtr<USentryTraceSampler> TraceSampler;'
        }

        # PowerShell 5.1 will write UT8-BOM if we use Out-File, so bypass this issue and use ::WriteAllLines
        [System.IO.File]::WriteAllLines("$projectRoot/package-release/Source/Sentry/Public/SentrySubsystem.h", $newSentrySubsystemHeader)

        Remove-Item -ErrorAction SilentlyContinue "$projectRoot/$packageName"

        # Workaround for Compress-Archive discarding file permissions
        # ======
        # Use of [System.IO.Compression.ZipFile]::CreateFromDirectory instead of Compress-Archive (or a third-party tool)
        # so that we retain file permissions
        # For more information, see https://github.com/PowerShell/Microsoft.PowerShell.Archive/issues/36
        # NOTE: This requires .NET 6+: https://github.com/dotnet/runtime/issues/1548
        Push-Location "$projectRoot/package-release"
        try
        {
            $location = Get-Location
            # Workaround in order to support LTS PowerShell version 5.1 on Windows
            if ([Environment]::GetEnvironmentVariable("OS") -Match "Windows")
            {
                Add-Type -AssemblyName "System.IO.Compression.FileSystem"
            }
            [System.IO.Compression.ZipFile]::CreateFromDirectory($location, "$projectRoot/$packageName")
        }
        finally
        {
            Pop-Location
        }
    }
}

packFiles
