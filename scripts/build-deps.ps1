# Builds plugin dependencies locally and replaces corresponding binaries in `plugin-dev/Sources/ThirdParty/...`
#
# Plugin dependencies:
# * sentry-native - for Windows support (can be built only on Windows)
# * sentry-cocoa - for Mac and iOS support (can be built only on MacOS)
# * sentry-java - for Android support (can be built both on Windows and MacOS)
#
# Usage:
#   .\build-deps.ps1 -All                                  # Build all SDKs for current platform
#   .\build-deps.ps1 -Cocoa -Java                          # Build only Cocoa and Java
#   .\build-deps.ps1 -Native -NativePath "C:\custom\path"  # Build Native with custom path
#   .\build-deps.ps1 -All -CocoaPath "C:\custom\path"      # Build all with custom Cocoa path
#
# Environment variables (used as fallback if custom paths not provided):
#   SENTRY_COCOA_PATH - Path to local sentry-cocoa repository
#   SENTRY_NATIVE_PATH - Path to local sentry-native repository
#   SENTRY_JAVA_PATH - Path to local sentry-java repository

param(
    [switch]$All,
    [switch]$Cocoa,
    [switch]$Native,
    [switch]$Java,
    [string]$CocoaPath,
    [string]$NativePath,
    [string]$JavaPath
)

Set-StrictMode -Version latest

# Compatibility: $IsWindows doesn't exist in Windows PowerShell 5.1 (only in PowerShell Core 6+)
# If not defined, we're on Windows PowerShell 5.1 which only runs on Windows
$isWindowsPlatform = if ($null -eq (Get-Variable -Name IsWindows -ErrorAction SilentlyContinue)) {
    $true  # Windows PowerShell 5.1 only runs on Windows
} else {
    $IsWindows
}

# If -All is specified, enable all SDKs for current platform
if ($All)
{
    if ($isWindowsPlatform)
    {
        $Native = $true
        $Java = $true
    }
    else
    {
        $Cocoa = $true
        $Java = $true
    }
}

# Check if at least one SDK is selected
if (-not ($Cocoa -or $Native -or $Java))
{
    Write-Host "Error: No SDK specified. Use -All or specify individual SDKs (-Cocoa, -Native, -Java)" -ForegroundColor Red
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\build-deps.ps1 -All"
    Write-Host "  .\build-deps.ps1 -Cocoa -Java"
    Write-Host "  .\build-deps.ps1 -Native -NativePath 'D:\projects\sentry-native'"
    Write-Host ""
    Write-Host "Environment variables (used as fallback):"
    Write-Host "  SENTRY_COCOA_PATH  - Path to local sentry-cocoa repository"
    Write-Host "  SENTRY_NATIVE_PATH - Path to local sentry-native repository"
    Write-Host "  SENTRY_JAVA_PATH   - Path to local sentry-java repository"
    exit 1
}

# Determine which SDKs to build based on flags
$buildCocoa = $Cocoa
$buildNative = $Native
$buildJava = $Java

# Resolve paths: use custom path if provided, otherwise fallback to environment variable
if ([string]::IsNullOrEmpty($CocoaPath)) { $CocoaPath = $env:SENTRY_COCOA_PATH }
if ([string]::IsNullOrEmpty($NativePath)) { $NativePath = $env:SENTRY_NATIVE_PATH }
if ([string]::IsNullOrEmpty($JavaPath)) { $JavaPath = $env:SENTRY_JAVA_PATH }

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

# Build SDKs based on flags
if ($buildCocoa)
{
    if ([string]::IsNullOrEmpty($CocoaPath))
    {
        Write-Warning "Cocoa SDK build requested but path is not set."
        Write-Warning "Provide -CocoaPath parameter or set SENTRY_COCOA_PATH environment variable."
    }
    else
    {
        buildSentryCocoaMac
        buildSentryCocoaIos
    }
}

if ($buildNative)
{
    if ([string]::IsNullOrEmpty($NativePath))
    {
        Write-Warning "Native SDK build requested but path is not set."
        Write-Warning "Provide -NativePath parameter or set SENTRY_NATIVE_PATH environment variable."
    }
    else
    {
        buildSentryNative
    }
}

if ($buildJava)
{
    if ([string]::IsNullOrEmpty($JavaPath))
    {
        Write-Warning "Java SDK build requested but path is not set."
        Write-Warning "Provide -JavaPath parameter or set SENTRY_JAVA_PATH environment variable."
    }
    else
    {
        buildSentryJava
    }
}
