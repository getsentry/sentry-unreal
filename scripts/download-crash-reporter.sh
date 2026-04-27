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
        local archiveName="Sentry.CrashReporter.exe"
        local destName="Sentry.CrashReporter.exe"
    elif [[ "$runtimeId" == osx-* ]]; then
        local archiveUrl="$baseUrl-$runtimeId.tar.gz"
        local archiveName="Sentry Crash Reporter.app"
        local destName="Sentry.CrashReporter.app"
    else
        local archiveUrl="$baseUrl-$runtimeId.tar.gz"
        local archiveName="Sentry.CrashReporter"
        local destName="Sentry.CrashReporter"
    fi

    echo "Downloading Crash Reporter for $platform ($runtimeId) ..."

    local tempDir
    tempDir=$(mktemp -d)

    if [[ "$runtimeId" == win-* ]]; then
        curl -sL "$archiveUrl" -o "$tempDir/archive.zip"
        unzip -qo "$tempDir/archive.zip" "$archiveName" -d "$tempDir"
    else
        curl -sL "$archiveUrl" -o "$tempDir/archive.tar.gz"
        tar -xzf "$tempDir/archive.tar.gz" -C "$tempDir"
    fi

    # Remove macOS resource fork files that GNU tar creates from code-signing xattrs
    find "$tempDir" -name '._*' -delete 2>/dev/null || true

    mkdir -p "$targetDir/$platform"
    mv "$tempDir/$archiveName" "$targetDir/$platform/$destName"

    if [[ -f "$targetDir/$platform/$destName" ]]; then
        chmod +x "$targetDir/$platform/$destName"
    fi

    rm -rf "$tempDir"
}

downloadCrashReporter "Win64"     "win-x64"
downloadCrashReporter "WinArm64"  "win-arm64"
downloadCrashReporter "Linux"     "linux-x64"
downloadCrashReporter "LinuxArm64" "linux-arm64"
downloadCrashReporter "Mac"       "osx-arm64"
