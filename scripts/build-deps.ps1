# Builds plugin dependencies locally and replaces corresponding binaries in `plugin-dev/Sources/ThirdParty/...`
#
# Plugin dependencies:
# * sentry-native - for Windows support (can be built only on Windows)
# * sentry-cocoa - for Mac and iOS support (can be built only on MacOS)
# * sentry-java - for Android support (can be built both on Windows and MacOS)
#
# Usage:
#   .\build-deps.ps1 -CocoaPath "C:\path\to\sentry-cocoa"  # Builds Mac & iOS only
#   .\build-deps.ps1 -NativePath "C:\path\to\sentry-native"  # Builds Windows only
#   .\build-deps.ps1 -JavaPath "C:\path\to\sentry-java"  # Builds Android only
#   .\build-deps.ps1 -CocoaPath "..." -JavaPath "..."  # Builds multiple SDKs
#   .\build-deps.ps1  # Uses environment variables (fallback)
#
# Environment variables (used as fallback if parameters not provided):
#   SENTRY_COCOA_PATH - Path to local sentry-cocoa repository
#   SENTRY_NATIVE_PATH - Path to local sentry-native repository
#   SENTRY_JAVA_PATH - Path to local sentry-java repository

param(
    [string]$CocoaPath,
    [string]$NativePath,
    [string]$JavaPath
)

# Fallback to environment variables if parameters not provided
if ([string]::IsNullOrEmpty($CocoaPath)) { $CocoaPath = $env:SENTRY_COCOA_PATH }
if ([string]::IsNullOrEmpty($NativePath)) { $NativePath = $env:SENTRY_NATIVE_PATH }
if ([string]::IsNullOrEmpty($JavaPath)) { $JavaPath = $env:SENTRY_JAVA_PATH }

Set-StrictMode -Version latest

$outDir = Resolve-Path "$PSScriptRoot/../plugin-dev/Source/ThirdParty"

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
    if ([string]::IsNullOrEmpty($CocoaPath))
    {
        Write-Warning "Cocoa path is not set. Skipping iOS build."
        Write-Warning "Provide -CocoaPath parameter or set SENTRY_COCOA_PATH environment variable."
        return
    }

    if (-not (Test-Path $CocoaPath))
    {
        throw "Sentry Cocoa path does not exist: $CocoaPath"
    }

    Write-Host "Building Sentry Cocoa for iOS using local repository at: $CocoaPath"

    Push-Location -Path $CocoaPath

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
    extractXCFramework "$CocoaPath/Carthage/Sentry-Dynamic.xcframework.zip" $tempExtractDir

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
    if ([string]::IsNullOrEmpty($CocoaPath))
    {
        Write-Warning "Cocoa path is not set. Skipping Mac build."
        Write-Warning "Provide -CocoaPath parameter or set SENTRY_COCOA_PATH environment variable."
        return
    }

    if (-not (Test-Path $CocoaPath))
    {
        throw "Sentry Cocoa path does not exist: $CocoaPath"
    }

    Write-Host "Building Sentry Cocoa for Mac using local repository at: $CocoaPath"

    Push-Location -Path $CocoaPath

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
    extractXCFramework "$CocoaPath/Carthage/Sentry-Dynamic.xcframework.zip" $tempExtractDir

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
    if ([string]::IsNullOrEmpty($JavaPath))
    {
        Write-Warning "Java path is not set. Skipping Android build."
        Write-Warning "Provide -JavaPath parameter or set SENTRY_JAVA_PATH environment variable."
        return
    }

    if (-not (Test-Path $JavaPath))
    {
        throw "Sentry Java path does not exist: $JavaPath"
    }

    Write-Host "Building Sentry Java for Android using local repository at: $JavaPath"

    Push-Location -Path $JavaPath

    ./gradlew -PsentryAndroidSdkName="sentry.native.android.unreal" `
        :sentry-android-core:assembleRelease :sentry-android-ndk:assembleRelease :sentry:jar --no-daemon --stacktrace --warning-mode none

    Pop-Location

    $androidOutDir = "$outDir/Android"

    if (Test-Path $androidOutDir)
    {
        Remove-Item $androidOutDir -Recurse -Force
    }

    New-Item $androidOutDir -ItemType Directory > $null

    Copy-Item "$JavaPath/sentry-android-ndk/build/outputs/aar/sentry-android-ndk-release.aar" -Destination "$androidOutDir/sentry-android-ndk-release.aar"
    Copy-Item "$JavaPath/sentry-android-core/build/outputs/aar/sentry-android-core-release.aar" -Destination "$androidOutDir/sentry-android-core-release.aar"
    Copy-Item "$JavaPath/sentry/build/libs/$("sentry-*.jar")" -Destination "$androidOutDir/sentry.jar"
}

function buildSentryNative()
{
    if ([string]::IsNullOrEmpty($NativePath))
    {
        Write-Warning "Native path is not set. Skipping Windows build."
        Write-Warning "Provide -NativePath parameter or set SENTRY_NATIVE_PATH environment variable."
        return
    }

    if (-not (Test-Path $NativePath))
    {
        throw "Sentry Native path does not exist: $NativePath"
    }

    Write-Host "Building Sentry Native for Windows using local repository at: $NativePath"

    Push-Location -Path $NativePath

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

    Get-ChildItem -Path "$NativePath/install/lib" -Filter "*.lib" -Recurse | Copy-Item -Destination $nativeOutDirLibs
    Copy-Item "$NativePath/install/bin/crashpad_handler.exe" -Destination $nativeOutDirBinaries
    Copy-Item "$NativePath/install/bin/crashpad_wer.dll" -Destination $nativeOutDirBinaries
    Copy-Item "$NativePath/install/include/sentry.h" -Destination $nativeOutDirIncludes
}

# Build SDKs based on provided paths
if (![string]::IsNullOrEmpty($CocoaPath))
{
    buildSentryCocoaMac
    buildSentryCocoaIos
}

if (![string]::IsNullOrEmpty($NativePath))
{
    buildSentryNative
}

if (![string]::IsNullOrEmpty($JavaPath))
{
    buildSentryJava
}
