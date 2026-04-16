#!/usr/bin/env bash
set -euo pipefail

echo "Downloading native SDKs from the latest CI pipeline"
cd "$(dirname $0)/../plugin-dev/Source/ThirdParty"

findCiRun() {
    echo "Looking for the latest successful CI run on branch '$1'" >/dev/stderr
    id=$(gh run list --branch $1 --workflow package-plugin-workflow \
        --json 'conclusion,databaseId' --jq 'first(.[] | select(.conclusion == "success") | .databaseId)')
    if [[ "$id" == "" ]]; then
        echo "  ... no successful CI run found on $1" >/dev/stderr
        return 1
    else
        echo "  ... found CI run ID: $id" >/dev/stderr
        echo "$id"
        return 0
    fi
}

runId=$(findCiRun "$(git rev-parse --abbrev-ref HEAD)" || findCiRun main)
if [[ "$runId" == "" ]]; then
    exit 1
fi

# Mobile platforms: single artifact per platform
declare -a otherSdks=("Android" "IOS")
for sdk in "${otherSdks[@]}"; do
    echo "Downloading $sdk SDK to $PWD/$sdk ..."
    rm -rf "./$sdk"
    gh run download $runId -n "$sdk-sdk" -D $sdk
done

# Mac: cocoa SDK goes into Mac/Cocoa, native SDK into Mac/Native
echo "Downloading Mac Cocoa SDK to $PWD/Mac/Cocoa ..."
rm -rf "./Mac/Cocoa"
gh run download $runId -n "Mac-cocoa-sdk" -D Mac

echo "Downloading Mac Native SDK to $PWD/Mac/Native ..."
rm -rf "./Mac/Native"
gh run download $runId -n "Mac-native-sdk" -D Mac/Native

# Native platforms: two backend variants per platform
declare -a nativePlatforms=("Linux" "LinuxArm64" "Win64" "WinArm64")
for platform in "${nativePlatforms[@]}"; do
    for backend in crashpad native; do
        backendDir="$(tr '[:lower:]' '[:upper:]' <<< "${backend:0:1}")${backend:1}"
        targetDir="$platform/$backendDir"
        echo "Downloading $platform-$backend SDK to $PWD/$targetDir ..."
        rm -rf "./$targetDir"
        gh run download $runId -n "$platform-$backend-sdk" -D "$targetDir"
    done
done

# Set permissions for Linux/Mac executables
for platform in Linux LinuxArm64; do
    chmod +x "$platform/Crashpad/bin/crashpad_handler"
    chmod +x "$platform/Native/bin/sentry-crash"
done
chmod +x Mac/Native/bin/sentry-crash

