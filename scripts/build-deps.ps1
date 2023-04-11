Set-StrictMode -Version latest

$modulesDir = Resolve-Path "$PSScriptRoot/../modules"
$outDir = Resolve-Path "$PSScriptRoot/../plugin-dev/Source/ThirdParty"

function buildSentryCocoaIos()
{
    Push-Location -Path "$modulesDir/sentry-cocoa"

    carthage build --use-xcframeworks --no-skip-current --platform iOS

    Pop-Location

    $iosOutDir = "$outDir/IOS"

    if (Test-Path $iosOutDir)
    {
        Remove-Item $iosOutDir -Recurse -Force
    }

    New-Item $iosOutDir -ItemType Directory > $null
    New-Item "$iosOutDir/Sentry.embeddedframework" -ItemType Directory > $null

    Copy-Item "$modulesDir/sentry-cocoa/Carthage/Build/Sentry.xcframework/ios-arm64/Sentry.framework" -Destination "$iosOutDir/Sentry.framework" -Recurse
    Copy-Item "$modulesDir/sentry-cocoa/Carthage/Build/Sentry.xcframework/ios-arm64/Sentry.framework" -Destination "$iosOutDir/Sentry.embeddedframework/Sentry.framework" -Recurse

    Push-Location $iosOutDir
    try
    {
        zip -r "Sentry.embeddedframework.zip" "Sentry.embeddedframework"
    }
    finally
    {
        Pop-Location
    }

    Remove-Item "$iosOutDir/Sentry.embeddedframework" -Recurse -Force
}

function buildSentryCocoaMac()
{
    Push-Location -Path "$modulesDir/sentry-cocoa"

    carthage build --no-skip-current --platform macOS

    Pop-Location

    $macOutDir = "$outDir/Mac"
    $macOutDirBinaries = "$macOutDir/bin"
    $macOutDirIncludes = "$macOutDir/include/Headers"

    if (Test-Path $macOutDir)
    {
        Remove-Item $macOutDir -Recurse -Force
    }

    New-Item $macOutDir -ItemType Directory > $null
    New-Item $macOutDirBinaries -ItemType Directory > $null
    New-Item $macOutDirIncludes -ItemType Directory > $null

    Copy-Item "$modulesDir/sentry-cocoa/Carthage/Build/Mac/Sentry.framework/Sentry" -Destination "$macOutDirBinaries/sentry.dylib"
    Copy-Item "$modulesDir/sentry-cocoa/Carthage/Build/Mac/Sentry.framework/Headers/*" -Destination $macOutDirIncludes
    Copy-Item "$modulesDir/sentry-cocoa/Carthage/Build/Mac/Sentry.framework/PrivateHeaders/*" -Destination $macOutDirIncludes
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

    cmake -B "build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal
    cmake --build "build" --target sentry --config RelWithDebInfo --parallel
    cmake --build "build" --target crashpad_handler --config Release --parallel

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

    Copy-Item "$modulesDir/sentry-native/build/RelWithDebInfo/sentry.lib" -Destination "$nativeOutDirLibs/sentry.lib"
    Copy-Item "$modulesDir/sentry-native/build/RelWithDebInfo/sentry.dll" -Destination "$nativeOutDirBinaries/sentry.dll"
    Copy-Item "$modulesDir/sentry-native/build/RelWithDebInfo/sentry.pdb" -Destination "$nativeOutDirBinaries/sentry.pdb"
    Copy-Item "$modulesDir/sentry-native/build/crashpad_build/handler/Release/crashpad_handler.exe" -Destination "$nativeOutDirBinaries/crashpad_handler.exe"
    Copy-Item "$modulesDir/sentry-native/include/sentry.h" -Destination "$nativeOutDirIncludes/sentry.h"
}

if ($IsMacOS)
{
    buildSentryCocoaMac
    buildSentryCocoaIos
    buildSentryJava
}
elseif ($IsWindows)
{
    buildSentryNative
    buildSentryJava
}