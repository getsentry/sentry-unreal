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

    # We know the file is meant to be UTF8, so let's be explicit
    $sentrySubsystemHeader = Get-Content "plugin-dev/Source/Sentry/Public/SentrySubsystem.h" -Encoding UTF8

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

        # Replacing raw pointers in UPROPERTY fields with TObjectPtr for UE 5.0+
        # See https://github.com/getsentry/sentry-unreal/issues/1082

        # Workaround for PowerShell 5.1 writing UTF8-BOM
        # ======
        # Set current directory so that ::WriteAllLines can accept a relative path
        [System.Environment]::CurrentDirectory = (Get-Location).Path

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
        [System.IO.File]::WriteAllLines("package-release/Source/Sentry/Public/SentrySubsystem.h", $newSentrySubsystemHeader)

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
