#!/usr/bin/env bash
set -euo pipefail

function getProperty {
    cat "$(dirname $0)/../modules/sentry-crash-reporter.properties" | grep $1 | cut -d'=' -f2 | tr -d '"'
}

baseUrl="$(getProperty 'repo')/releases/download/$(getProperty 'version')/sentry-desktop-crash-reporter-$(getProperty 'version')"
targetDir="$(dirname $0)/../plugin-dev/Source/ThirdParty"

downloadCrashReporter() {
    local platform="$1"
    local runtimeId="$2"

    if [[ "$runtimeId" == win-* ]]; then
        local archiveUrl="$baseUrl-$runtimeId.zip"
        local executableName="Sentry.CrashReporter.exe"
    else
        local archiveUrl="$baseUrl-$runtimeId.tar.gz"
        local executableName="Sentry.CrashReporter"
    fi

    echo "Downloading Crash Reporter for $platform ($runtimeId) ..."

    local tempDir
    tempDir=$(mktemp -d)

    if [[ "$runtimeId" == win-* ]]; then
        curl -sL "$archiveUrl" -o "$tempDir/archive.zip"
        unzip -qo "$tempDir/archive.zip" "$executableName" -d "$tempDir"
    else
        curl -sL "$archiveUrl" -o "$tempDir/archive.tar.gz"
        tar -xzf "$tempDir/archive.tar.gz" -C "$tempDir" "./$executableName"
    fi

    mkdir -p "$targetDir/$platform"
    mv "$tempDir/$executableName" "$targetDir/$platform/$executableName"
    chmod +x "$targetDir/$platform/$executableName"

    rm -rf "$tempDir"
}

downloadCrashReporter "Win64"     "win-x64"
downloadCrashReporter "WinArm64"  "win-arm64"
downloadCrashReporter "Linux"     "linux-x64"
downloadCrashReporter "LinuxArm64" "linux-arm64"
downloadCrashReporter "Mac"       "osx-arm64"
