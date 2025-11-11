# Builds plugin dependencies locally and replaces corresponding binaries in `plugin-dev/Sources/ThirdParty/...`
# Depending on a platform this script is running on some dependencies may or may not be built.

# Plugin dependencies:
# * sentry-native - for Windows support (can be built only on Windows)
# * sentry-cocoa - for Mac and iOS support (can be built only on MacOS)
#   - Requires SENTRY_COCOA_PATH environment variable pointing to local sentry-cocoa repository
#   - Example: $env:SENTRY_COCOA_PATH="/path/to/sentry-cocoa"; ./build-deps.ps1 mac ios
# * sentry-java - for Android support (can be built both on Windows and MacOS)

# Running this script without parameters will build all plugin dependencies it can on the current platform.

# To build only a certain plugin dependency run `pwsh ./build-deps.ps1 <platfrom_name>`.
# Supported platforms names are: `win`, `mac`, `ios`, `android`.

param(
    [Parameter(ValueFromRemainingArguments=$true)]
    [string[]]$Platforms
)

$SentryCocoaPath = $env:SENTRY_COCOA_PATH

Set-StrictMode -Version latest

$modulesDir = Resolve-Path "$PSScriptRoot/../modules"
$outDir = Resolve-Path "$PSScriptRoot/../plugin-dev/Source/ThirdParty"

$macPlatfromDeps = @("mac", "ios", "android")
$winPlatfromDeps = @("win", "android")

function extractXCFramework([string] $zipPath, [string] $destination)
{
    if (-not (Test-Path $zipPath))
    {
        throw "XCFramework zip not found at: $zipPath"
    }

    if (Test-Path $destination)
    {
        Remove-Item $destination -Recurse -Force
    }

    New-Item $destination -ItemType Directory > $null

    Push-Location $destination
    try
    {
        unzip -q "$zipPath"
    }
    finally
    {
        Pop-Location
    }
}

function buildSentryCocoaIos()
{
    if ([string]::IsNullOrEmpty($SentryCocoaPath))
    {
        Write-Warning "SENTRY_COCOA_PATH environment variable is not set. Skipping iOS build."
        Write-Warning "Set it to your local sentry-cocoa repository path: `$env:SENTRY_COCOA_PATH='/path/to/sentry-cocoa'"
        return
    }

    if (-not (Test-Path $SentryCocoaPath))
    {
        throw "Sentry Cocoa path does not exist: $SentryCocoaPath"
    }

    Write-Host "Building Sentry Cocoa for iOS using local repository at: $SentryCocoaPath"

    Push-Location -Path $SentryCocoaPath

    try
    {
        # Build dynamic XCFramework for iOS only
        bash ./scripts/build-xcframework-local.sh "iOSOnly" "DynamicOnly"

        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build Sentry Cocoa XCFramework for iOS"
        }
    }
    finally
    {
        Pop-Location
    }

    # Extract the built XCFramework
    $tempExtractDir = "$PSScriptRoot/../build/temp-xcframework-ios"
    extractXCFramework "$SentryCocoaPath/Carthage/Sentry-Dynamic.xcframework.zip" $tempExtractDir

    # Prepare output directories
    $iosOutDir = "$outDir/IOS"

    if (Test-Path $iosOutDir)
    {
        Remove-Item $iosOutDir -Recurse -Force
    }

    New-Item $iosOutDir -ItemType Directory > $null

    # Copy iOS framework
    Copy-Item "$tempExtractDir/Sentry-Dynamic.xcframework/ios-arm64/Sentry.framework" -Destination "$iosOutDir/Sentry.framework" -Recurse

    # Create embedded framework structure
    New-Item "$iosOutDir/Sentry.embeddedframework" -ItemType Directory > $null
    Copy-Item "$tempExtractDir/Sentry-Dynamic.xcframework/ios-arm64/Sentry.framework" -Destination "$iosOutDir/Sentry.embeddedframework/Sentry.framework" -Recurse

    # Create zip for embedded framework
    Push-Location $iosOutDir
    try
    {
        zip -r "Sentry.embeddedframework.zip" "Sentry.embeddedframework"
    }
    finally
    {
        Pop-Location
    }

    # Cleanup
    Remove-Item "$iosOutDir/Sentry.embeddedframework" -Recurse -Force
    Remove-Item $tempExtractDir -Recurse -Force

    Write-Host "Successfully built Sentry Cocoa for iOS"
}

function buildSentryCocoaMac()
{
    if ([string]::IsNullOrEmpty($SentryCocoaPath))
    {
        Write-Warning "SENTRY_COCOA_PATH environment variable is not set. Skipping Mac build."
        Write-Warning "Set it to your local sentry-cocoa repository path: `$env:SENTRY_COCOA_PATH='/path/to/sentry-cocoa'"
        return
    }

    if (-not (Test-Path $SentryCocoaPath))
    {
        throw "Sentry Cocoa path does not exist: $SentryCocoaPath"
    }

    Write-Host "Building Sentry Cocoa for Mac using local repository at: $SentryCocoaPath"

    Push-Location -Path $SentryCocoaPath

    try
    {
        # Build dynamic XCFramework for Mac only
        bash ./scripts/build-xcframework-local.sh "macOSOnly" "DynamicOnly"

        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build Sentry Cocoa XCFramework for Mac"
        }
    }
    finally
    {
        Pop-Location
    }

    # Extract the built XCFramework
    $tempExtractDir = "$PSScriptRoot/../build/temp-xcframework-mac"
    extractXCFramework "$SentryCocoaPath/Carthage/Sentry-Dynamic.xcframework.zip" $tempExtractDir

    # Prepare output directories
    $macOutDir = "$outDir/Mac"
    $macOutDirBinaries = "$macOutDir/bin"
    $macOutDirIncludes = "$macOutDir/include/Sentry"

    if (Test-Path $macOutDir)
    {
        Remove-Item $macOutDir -Recurse -Force
    }

    New-Item $macOutDir -ItemType Directory > $null
    New-Item $macOutDirBinaries -ItemType Directory > $null
    New-Item $macOutDirIncludes -ItemType Directory > $null

    # Copy Mac framework binary
    Copy-Item "$tempExtractDir/Sentry-Dynamic.xcframework/macos-arm64_x86_64/Sentry.framework/Sentry" -Destination "$macOutDirBinaries/sentry.dylib"

    # Copy headers and private headers
    Copy-Item "$tempExtractDir/Sentry-Dynamic.xcframework/macos-arm64_x86_64/Sentry.framework/Headers/*" -Destination $macOutDirIncludes
    Copy-Item "$tempExtractDir/Sentry-Dynamic.xcframework/macos-arm64_x86_64/Sentry.framework/PrivateHeaders/*" -Destination $macOutDirIncludes

    # Cleanup
    Remove-Item $tempExtractDir -Recurse -Force

    Write-Host "Successfully built Sentry Cocoa for Mac"
}

function buildSentryJava()
{
    Push-Location -Path "$modulesDir/sentry-java"

    ./gradlew -PsentryAndroidSdkName="sentry.native.android.unreal" `
        :sentry-android-core:assembleRelease :sentry-android-ndk:assembleRelease :sentry:jar --no-daemon --stacktrace --warning-mode none

    Pop-Location

    $androidOutDir = "$outDir/Android"

    if (Test-Path $androidOutDir)
    {
        Remove-Item $androidOutDir -Recurse -Force
    }

    New-Item $androidOutDir -ItemType Directory > $null

    Copy-Item "$modulesDir/sentry-java/sentry-android-ndk/build/outputs/aar/sentry-android-ndk-release.aar" -Destination "$androidOutDir/sentry-android-ndk-release.aar"
    Copy-Item "$modulesDir/sentry-java/sentry-android-core/build/outputs/aar/sentry-android-core-release.aar" -Destination "$androidOutDir/sentry-android-core-release.aar"
    Copy-Item "$modulesDir/sentry-java/sentry/build/libs/$("sentry-*.jar")" -Destination "$androidOutDir/sentry.jar"
}

function buildSentryNative()
{
    Push-Location -Path "$modulesDir/sentry-native"

    cmake -B "build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF
    cmake --build "build" --target sentry --config RelWithDebInfo --parallel
    cmake --build "build" --target crashpad_handler --config RelWithDebInfo --parallel
    cmake --install "build" --prefix "install" --config RelWithDebInfo

    Pop-Location

    $nativeOutDir = "$outDir/Win64"
    $nativeOutDirLibs = "$nativeOutDir/lib"
    $nativeOutDirBinaries = "$nativeOutDir/bin"
    $nativeOutDirIncludes = "$nativeOutDir/include"

    if (Test-Path $nativeOutDir)
    {
        Remove-Item $nativeOutDir -Recurse -Force
    }

    New-Item $nativeOutDir -ItemType Directory > $null
    New-Item $nativeOutDirLibs -ItemType Directory > $null
    New-Item $nativeOutDirBinaries -ItemType Directory > $null
    New-Item $nativeOutDirIncludes -ItemType Directory > $null

    Get-ChildItem -Path "$modulesDir/sentry-native/install/lib" -Filter "*.lib" -Recurse | Copy-Item -Destination $nativeOutDirLibs
    Copy-Item "$modulesDir/sentry-native/install/bin/crashpad_handler.exe" -Destination $nativeOutDirBinaries
    Copy-Item "$modulesDir/sentry-native/install/bin/crashpad_wer.dll" -Destination $nativeOutDirBinaries
    Copy-Item "$modulesDir/sentry-native/install/include/sentry.h" -Destination $nativeOutDirIncludes
}

function buildPlatformDependency([string] $platform)
{
    if ($platform -eq "win")
    {
        buildSentryNative
    }
    elseif ($platform -eq "mac")
    {
        buildSentryCocoaMac
    }
    elseif ($platform -eq "ios")
    {
        buildSentryCocoaIos
    }
    elseif ($platform -eq "android")
    {
        buildSentryJava
    }
    else
    {
        Write-Warning "Platform '$platform' is not supported"
    }
}

if ($null -eq $Platforms -or $Platforms.Count -eq 0)
{
    $platformsToBuild = if ($IsMacOS) { $macPlatfromDeps } else { $winPlatfromDeps }
    foreach ($platform in $platformsToBuild)
    {
        buildPlatformDependency($platform)
    }
}
else
{
    foreach ($platform in $Platforms)
    {
        buildPlatformDependency($platform)
    }
}
