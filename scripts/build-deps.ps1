# Builds plugin dependencies locally and replaces corresponding binaries in `plugin-dev/Sources/ThirdParty/...`
#
# Plugin dependencies:
# * sentry-native - for Windows/Linux/Mac support
# * sentry-cocoa - for Mac and iOS support (can be built only on MacOS)
# * sentry-java - for Android support
# * sentry-crash-reporter - external crash reporter
#
# Usage:
#   .\build-deps.ps1 -All                                  # Build all SDKs for current platform
#   .\build-deps.ps1 -Cocoa -Java                          # Build only Cocoa and Java
#   .\build-deps.ps1 -Native -NativePath "C:\custom\path"  # Build Native with custom path
#   .\build-deps.ps1 -All -CocoaPath "C:\custom\path"      # Build all with custom Cocoa path
#   .\build-deps.ps1 -CrashReporter                        # Build crash reporter
#
# Environment variables (used as fallback if custom paths not provided):
#   SENTRY_COCOA_PATH - Path to local sentry-cocoa repository
#   SENTRY_NATIVE_PATH - Path to local sentry-native repository
#   SENTRY_JAVA_PATH - Path to local sentry-java repository
#   SENTRY_CRASH_REPORTER_PATH - Path to local sentry-crash-reporter repository

param(
    [switch]$All,
    [switch]$Cocoa,
    [switch]$Native,
    [switch]$Java,
    [switch]$CrashReporter,
    [string]$CocoaPath,
    [string]$NativePath,
    [string]$JavaPath,
    [string]$CrashReporterPath
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
        $CrashReporter = $true
    }
    elseif ($IsLinux)
    {
        $Native = $true
        $Java = $true
        $CrashReporter = $true
    }
    else
    {
        $Cocoa = $true
        $Native = $true
        $Java = $true
        $CrashReporter = $true
    }
}

# Check if at least one SDK is selected
if (-not ($Cocoa -or $Native -or $Java -or $CrashReporter))
{
    Write-Host "Error: No SDK specified. Use -All or specify individual SDKs (-Cocoa, -Native, -Java, -CrashReporter)" -ForegroundColor Red
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\build-deps.ps1 -All"
    Write-Host "  .\build-deps.ps1 -Cocoa -Java"
    Write-Host "  .\build-deps.ps1 -Native -NativePath 'D:\projects\sentry-native'"
    Write-Host "  .\build-deps.ps1 -CrashReporter"
    Write-Host ""
    Write-Host "Environment variables (used as fallback):"
    Write-Host "  SENTRY_COCOA_PATH           - Path to local sentry-cocoa repository"
    Write-Host "  SENTRY_NATIVE_PATH          - Path to local sentry-native repository"
    Write-Host "  SENTRY_JAVA_PATH            - Path to local sentry-java repository"
    Write-Host "  SENTRY_CRASH_REPORTER_PATH  - Path to local sentry-crash-reporter repository"
    exit 1
}

# Determine which SDKs to build based on flags
$buildCocoa = $Cocoa
$buildNative = $Native
$buildJava = $Java
$buildCrashReporter = $CrashReporter

# Resolve paths: use custom path if provided, otherwise fallback to environment variable
if ([string]::IsNullOrEmpty($CocoaPath)) { $CocoaPath = $env:SENTRY_COCOA_PATH }
if ([string]::IsNullOrEmpty($NativePath)) { $NativePath = $env:SENTRY_NATIVE_PATH }
if ([string]::IsNullOrEmpty($JavaPath)) { $JavaPath = $env:SENTRY_JAVA_PATH }
if ([string]::IsNullOrEmpty($CrashReporterPath)) { $CrashReporterPath = $env:SENTRY_CRASH_REPORTER_PATH }

$outDir = Resolve-Path "$PSScriptRoot/../plugin-dev/Source/ThirdParty"

function buildSentryCocoa()
{
    if (-not (Test-Path $CocoaPath))
    {
        throw "Sentry Cocoa path does not exist: $CocoaPath"
    }

    Write-Host "Building SentryObjC-Dynamic XCFramework (iOS + macOS) using local repository at: $CocoaPath"

    Push-Location -Path $CocoaPath

    try
    {
        make build-xcframework-sentryobjc-dynamic SDKS=iphoneos,macosx

        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build SentryObjC-Dynamic XCFramework"
        }
    }
    finally
    {
        Pop-Location
    }

    $xcframeworkPath = "$CocoaPath/SentryObjC-Dynamic.xcframework"
    if (-not (Test-Path $xcframeworkPath))
    {
        throw "SentryObjC-Dynamic XCFramework not found at: $xcframeworkPath"
    }

    # iOS artifacts
    $iosOutDir = "$outDir/IOS"

    if (Test-Path $iosOutDir)
    {
        Remove-Item $iosOutDir -Recurse -Force
    }

    New-Item $iosOutDir -ItemType Directory > $null

    Copy-Item "$xcframeworkPath/ios-arm64/SentryObjC.framework" -Destination "$iosOutDir/SentryObjC.framework" -Recurse

    New-Item "$iosOutDir/SentryObjC.embeddedframework" -ItemType Directory > $null
    Copy-Item "$xcframeworkPath/ios-arm64/SentryObjC.framework" -Destination "$iosOutDir/SentryObjC.embeddedframework/SentryObjC.framework" -Recurse

    Push-Location $iosOutDir
    try
    {
        zip -r "SentryObjC.embeddedframework.zip" "SentryObjC.embeddedframework"
    }
    finally
    {
        Pop-Location
    }

    Remove-Item "$iosOutDir/SentryObjC.embeddedframework" -Recurse -Force

    Write-Host "Successfully built Sentry Cocoa for iOS"

    # macOS artifacts
    $macCocoaDir = "$outDir/Mac/Cocoa"
    $macOutDirBinaries = "$macCocoaDir/bin"
    $macOutDirIncludes = "$macCocoaDir/include/SentryObjC"

    if (Test-Path $macCocoaDir)
    {
        Remove-Item $macCocoaDir -Recurse -Force
    }

    New-Item $macCocoaDir -ItemType Directory > $null
    New-Item $macOutDirBinaries -ItemType Directory > $null
    New-Item $macOutDirIncludes -ItemType Directory > $null

    Copy-Item "$xcframeworkPath/macos-arm64_x86_64/SentryObjC.framework/SentryObjC" -Destination "$macOutDirBinaries/SentryObjC.dylib"

    Copy-Item "$xcframeworkPath/macos-arm64_x86_64/SentryObjC.framework/Headers/*" -Destination $macOutDirIncludes

    Write-Host "Successfully built Sentry Cocoa for Mac"
}

function buildSentryJava()
{
    if (-not (Test-Path $JavaPath))
    {
        throw "Sentry Java path does not exist: $JavaPath"
    }

    Write-Host "Building Sentry Java for Android using local repository at: $JavaPath"

    # Gradle build is platform-agnostic (only needs a JDK); pick the wrapper for the current OS
    $gradlew = if ($isWindowsPlatform) { ".\gradlew.bat" } else { "./gradlew" }

    Push-Location -Path $JavaPath

    try
    {
        & $gradlew -PsentryAndroidSdkName="sentry.native.android.unreal" `
            :sentry-android-core:assembleRelease :sentry-android-ndk:assembleRelease :sentry-android-replay:assembleRelease :sentry:jar --no-daemon --stacktrace --warning-mode none

        if ($LASTEXITCODE -ne 0)
        {
            throw "Failed to build Sentry Java"
        }
    }
    finally
    {
        Pop-Location
    }

    $androidOutDir = "$outDir/Android"

    if (Test-Path $androidOutDir)
    {
        Remove-Item $androidOutDir -Recurse -Force
    }

    New-Item $androidOutDir -ItemType Directory > $null

    Copy-Item "$JavaPath/sentry-android-ndk/build/outputs/aar/sentry-android-ndk-release.aar" -Destination "$androidOutDir/sentry-android-ndk-release.aar"
    Copy-Item "$JavaPath/sentry-android-core/build/outputs/aar/sentry-android-core-release.aar" -Destination "$androidOutDir/sentry-android-core-release.aar"
    Copy-Item "$JavaPath/sentry-android-replay/build/outputs/aar/sentry-android-replay-release.aar" -Destination "$androidOutDir/sentry-android-replay-release.aar"
    Copy-Item "$JavaPath/sentry/build/libs/$("sentry-*.jar")" -Destination "$androidOutDir/sentry.jar"

    # With version v8 of the sentry-java the Native SDK NDK has to be downloaded separately from the sentry-native repo release page
    $configFile = "$JavaPath/gradle/libs.versions.toml"
    if (-not (Test-Path $configFile))
    {
        throw "libs.versions.toml file not found at $configFile"
    }

    $tomlContent = Get-Content $configFile -Raw
    if ($tomlContent -notmatch 'sentry-native-ndk[^\n]*version\s*=\s*"([^"]+)"')
    {
        throw "Failed to extract Native SDK NDK version from $configFile"
    }
    $nativeNdkVersion = $Matches[1]
    Write-Host "Extracted Sentry Native NDK version: $nativeNdkVersion"

    $nativeNdkCache = "$JavaPath/native-ndk-cache"
    if (-not (Test-Path $nativeNdkCache))
    {
        New-Item $nativeNdkCache -ItemType Directory > $null
    }

    $nativeNdkZip = "$nativeNdkCache/sentry-native-ndk-$nativeNdkVersion.zip"
    $nativeNdkUrl = "https://github.com/getsentry/sentry-native/releases/download/$nativeNdkVersion/sentry-native-ndk-$nativeNdkVersion.zip"

    if (-not (Test-Path $nativeNdkZip))
    {
        Invoke-WebRequest -Uri $nativeNdkUrl -OutFile $nativeNdkZip
    }

    Expand-Archive -Path $nativeNdkZip -DestinationPath $nativeNdkCache -Force

    Copy-Item "$nativeNdkCache/sentry-native-ndk-$nativeNdkVersion/sentry-native-ndk-release.aar" -Destination "$androidOutDir/sentry-native-ndk-release.aar"
}

function buildSentryNative()
{
    if (-not (Test-Path $NativePath))
    {
        throw "Sentry Native path does not exist: $NativePath"
    }

    Write-Host "Building Sentry Native for Windows using local repository at: $NativePath"

    $platformDir = "$outDir/Win64"

    # Build Crashpad backend
    Write-Host "Building Crashpad backend..."
    Push-Location -Path $NativePath
    try
    {
        cmake -B "build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF
        cmake --build "build" --target sentry --config RelWithDebInfo --parallel
        cmake --build "build" --target crashpad_handler --config RelWithDebInfo --parallel
        cmake --install "build" --prefix "install" --config RelWithDebInfo
    }
    finally
    {
        Pop-Location
    }

    $crashpadDir = "$platformDir/Crashpad"
    if (Test-Path $crashpadDir) { Remove-Item $crashpadDir -Recurse -Force }
    New-Item "$crashpadDir/lib" -ItemType Directory -Force > $null
    New-Item "$crashpadDir/bin" -ItemType Directory -Force > $null
    New-Item "$crashpadDir/include" -ItemType Directory -Force > $null

    Get-ChildItem -Path "$NativePath/install/lib" -Filter "*.lib" -Recurse | Copy-Item -Destination "$crashpadDir/lib"
    Copy-Item "$NativePath/install/bin/crashpad_handler.exe" -Destination "$crashpadDir/bin"
    Copy-Item "$NativePath/install/bin/crashpad_wer.dll" -Destination "$crashpadDir/bin"
    Copy-Item "$NativePath/install/include/sentry.h" -Destination "$crashpadDir/include"

    # Build Native backend
    Write-Host "Building Native backend..."
    Push-Location -Path $NativePath
    try
    {
        cmake -B "build_native" -D SENTRY_BACKEND=native -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF
        cmake --build "build_native" --target sentry --config RelWithDebInfo --parallel
        cmake --build "build_native" --target sentry-crash --config RelWithDebInfo --parallel
        cmake --install "build_native" --prefix "install_native" --config RelWithDebInfo
    }
    finally
    {
        Pop-Location
    }

    $nativeDir = "$platformDir/Native"
    if (Test-Path $nativeDir) { Remove-Item $nativeDir -Recurse -Force }
    New-Item "$nativeDir/lib" -ItemType Directory -Force > $null
    New-Item "$nativeDir/bin" -ItemType Directory -Force > $null
    New-Item "$nativeDir/include" -ItemType Directory -Force > $null

    Get-ChildItem -Path "$NativePath/install_native/lib" -Filter "*.lib" -Recurse | Copy-Item -Destination "$nativeDir/lib"
    Copy-Item "$NativePath/install_native/bin/sentry-crash.exe" -Destination "$nativeDir/bin"
    Copy-Item "$NativePath/install_native/include/sentry.h" -Destination "$nativeDir/include"
}

function buildSentryNativeMac()
{
    if (-not (Test-Path $NativePath))
    {
        throw "Sentry Native path does not exist: $NativePath"
    }

    Write-Host "Building Sentry Native for macOS using local repository at: $NativePath"

    $platformDir = "$outDir/Mac"

    # Build Native backend (universal binary: x86_64 + arm64)
    Write-Host "Building Native backend..."
    Push-Location -Path $NativePath
    try
    {
        cmake -B "build_native" -D SENTRY_BACKEND=native -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF `
            -D CMAKE_BUILD_TYPE=RelWithDebInfo -D "CMAKE_OSX_ARCHITECTURES=x86_64;arm64" -D CMAKE_OSX_DEPLOYMENT_TARGET=13.0
        cmake --build "build_native" --target sentry --parallel
        cmake --build "build_native" --target sentry-crash --parallel
        cmake --install "build_native" --prefix "install_native"
    }
    finally
    {
        Pop-Location
    }

    $nativeDir = "$platformDir/Native"
    if (Test-Path $nativeDir) { Remove-Item $nativeDir -Recurse -Force }
    New-Item "$nativeDir/lib" -ItemType Directory -Force > $null
    New-Item "$nativeDir/bin" -ItemType Directory -Force > $null
    New-Item "$nativeDir/include" -ItemType Directory -Force > $null

    Get-ChildItem -Path "$NativePath/install_native/lib" -Filter "*.a" -Recurse | Copy-Item -Destination "$nativeDir/lib"
    Copy-Item "$NativePath/build_native/sentry-crash" -Destination "$nativeDir/bin"
    Copy-Item "$NativePath/install_native/include/sentry.h" -Destination "$nativeDir/include"

    Write-Host "Successfully built Sentry Native for macOS"
}

function buildSentryNativeLinux()
{
    if (-not (Test-Path $NativePath))
    {
        throw "Sentry Native path does not exist: $NativePath"
    }

    Write-Host "Building Sentry Native for Linux (x64) using local repository at: $NativePath"

    $platformDir = "$outDir/Linux"

    # sentry-native is built with clang and libc++ for the static libs, but the crash handler
    # executables (crashpad_handler / sentry-crash) are built with libstdc++ to match Unreal.
    $clangC = "clang"
    $clangCxx = "clang++"
    if (-not (Get-Command $clangC -ErrorAction SilentlyContinue))
    {
        throw "clang not found. Install clang (with libc++) to build Sentry Native for Linux."
    }

    # Build Crashpad backend
    Write-Host "Building Crashpad backend..."
    Push-Location -Path $NativePath
    try
    {
        # Static libs with libc++
        cmake -B "build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF `
            -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_C_COMPILER=$clangC -D CMAKE_CXX_COMPILER=$clangCxx `
            -D CMAKE_CXX_FLAGS="-stdlib=libc++" -D CMAKE_EXE_LINKER_FLAGS="-stdlib=libc++" -D HAVE_COPY_FILE_RANGE=0
        cmake --build "build" --target sentry --parallel
        cmake --install "build" --prefix "install"

        # crashpad_handler executable with libstdc++ (no transport needed)
        cmake -B "build_crashpad_handler" -D SENTRY_BACKEND=crashpad -D SENTRY_TRANSPORT=none -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF `
            -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_C_COMPILER=$clangC -D CMAKE_CXX_COMPILER=$clangCxx `
            -D CMAKE_CXX_FLAGS="-stdlib=libstdc++" -D CMAKE_EXE_LINKER_FLAGS="-stdlib=libstdc++" -D HAVE_COPY_FILE_RANGE=0
        cmake --build "build_crashpad_handler" --target sentry --parallel
        cmake --install "build_crashpad_handler" --prefix "install_crashpad_handler"

        # Replace crashpad_handler from the first build with the libstdc++ one
        Copy-Item "$NativePath/install_crashpad_handler/bin/crashpad_handler" -Destination "$NativePath/install/bin/crashpad_handler" -Force
    }
    finally
    {
        Pop-Location
    }

    $crashpadDir = "$platformDir/Crashpad"
    if (Test-Path $crashpadDir) { Remove-Item $crashpadDir -Recurse -Force }
    New-Item "$crashpadDir/lib" -ItemType Directory -Force > $null
    New-Item "$crashpadDir/bin" -ItemType Directory -Force > $null
    New-Item "$crashpadDir/include" -ItemType Directory -Force > $null

    Get-ChildItem -Path "$NativePath/install/lib" -Filter "*.a" -Recurse | Copy-Item -Destination "$crashpadDir/lib"
    strip -x "$NativePath/install/bin/crashpad_handler" -o "$crashpadDir/bin/crashpad_handler"
    Copy-Item "$NativePath/install/include/sentry.h" -Destination "$crashpadDir/include"

    # Build Native backend
    Write-Host "Building Native backend..."
    Push-Location -Path $NativePath
    try
    {
        # Static libs with libc++
        cmake -B "build_native" -D SENTRY_BACKEND=native -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF `
            -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_C_COMPILER=$clangC -D CMAKE_CXX_COMPILER=$clangCxx `
            -D CMAKE_CXX_FLAGS="-stdlib=libc++" -D CMAKE_EXE_LINKER_FLAGS="-stdlib=libc++" -D HAVE_COPY_FILE_RANGE=0
        cmake --build "build_native" --target sentry --parallel
        cmake --install "build_native" --prefix "install_native"

        # sentry-crash executable with libstdc++ (needs transport to send crash envelopes)
        cmake -B "build_native_crash" -D SENTRY_BACKEND=native -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF `
            -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_C_COMPILER=$clangC -D CMAKE_CXX_COMPILER=$clangCxx `
            -D CMAKE_CXX_FLAGS="-stdlib=libstdc++" -D CMAKE_EXE_LINKER_FLAGS="-stdlib=libstdc++" -D HAVE_COPY_FILE_RANGE=0
        cmake --build "build_native_crash" --target sentry-crash --parallel
    }
    finally
    {
        Pop-Location
    }

    $nativeDir = "$platformDir/Native"
    if (Test-Path $nativeDir) { Remove-Item $nativeDir -Recurse -Force }
    New-Item "$nativeDir/lib" -ItemType Directory -Force > $null
    New-Item "$nativeDir/bin" -ItemType Directory -Force > $null
    New-Item "$nativeDir/include" -ItemType Directory -Force > $null

    Get-ChildItem -Path "$NativePath/install_native/lib" -Filter "*.a" -Recurse | Copy-Item -Destination "$nativeDir/lib"
    strip -x "$NativePath/build_native_crash/sentry-crash" -o "$nativeDir/bin/sentry-crash"
    Copy-Item "$NativePath/install_native/include/sentry.h" -Destination "$nativeDir/include"

    Write-Host "Successfully built Sentry Native for Linux"
}

function buildSentryCrashReporter()
{
    if (-not (Test-Path $CrashReporterPath))
    {
        throw "Sentry Crash Reporter path does not exist: $CrashReporterPath"
    }

    Write-Host "Building Sentry Crash Reporter using local repository at: $CrashReporterPath"

    if ($isWindowsPlatform)
    {
        $runtimeId = "win-x64"
        $platformDir = "Win64"
        $executableName = "Sentry.CrashReporter.exe"
    }
    elseif ($IsLinux)
    {
        $arch = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture
        if ($arch -eq [System.Runtime.InteropServices.Architecture]::Arm64)
        {
            $runtimeId = "linux-arm64"
            $platformDir = "LinuxArm64"
        }
        else
        {
            $runtimeId = "linux-x64"
            $platformDir = "Linux"
        }
        $executableName = "Sentry.CrashReporter"
    }
    else
    {
        $runtimeId = "osx-arm64"
        $platformDir = "Mac"
        $executableName = "Sentry.CrashReporter.app"
    }

    $tempDir = Join-Path ([System.IO.Path]::GetTempPath()) "sentry-crash-reporter-build"

    if (Test-Path $tempDir)
    {
        Remove-Item $tempDir -Recurse -Force
    }

    dotnet publish `
        -f net10.0-desktop `
        -r $runtimeId `
        "$CrashReporterPath/Sentry.CrashReporter/Sentry.CrashReporter.csproj" `
        -o $tempDir

    if ($LASTEXITCODE -ne 0)
    {
        throw "Failed to build Sentry Crash Reporter"
    }

    $destDir = "$outDir/$platformDir"
    New-Item $destDir -ItemType Directory -Force > $null

    if ($IsMacOS)
    {
        $srcAppBundle = Get-ChildItem -Path $tempDir -Filter "*.app" -Directory | Select-Object -First 1
        $destAppBundle = "$destDir/$executableName"
        if (Test-Path $destAppBundle)
        {
            Remove-Item $destAppBundle -Recurse -Force
        }
        Copy-Item $srcAppBundle.FullName -Destination $destAppBundle -Recurse
    }
    else
    {
        Copy-Item "$tempDir/$executableName" -Destination "$destDir/$executableName"
    }

    # Cleanup
    Remove-Item $tempDir -Recurse -Force

    Write-Host "Successfully built Sentry Crash Reporter for $platformDir"
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
        buildSentryCocoa
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
        if ($isWindowsPlatform)
        {
            buildSentryNative
        }
        elseif ($IsMacOS)
        {
            buildSentryNativeMac
        }
        elseif ($IsLinux)
        {
            buildSentryNativeLinux
        }
        else
        {
            Write-Warning "Native SDK build is not supported on this platform from build-deps.ps1. Use the platform-specific build script instead."
        }
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

if ($buildCrashReporter)
{
    if ([string]::IsNullOrEmpty($CrashReporterPath))
    {
        Write-Warning "Crash Reporter build requested but path is not set."
        Write-Warning "Provide -CrashReporterPath parameter or set SENTRY_CRASH_REPORTER_PATH environment variable."
    }
    else
    {
        buildSentryCrashReporter
    }
}
