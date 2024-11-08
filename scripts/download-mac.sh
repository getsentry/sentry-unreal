#!/bin/bash
set -euo pipefail

function getProperty {
    cat "$(dirname $0)/../modules/sentry-cocoa.properties" | grep $1 | cut -d'=' -f2 | tr -d '"'
}

export sentryCocoaCache=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

cocoaRepo=$(getProperty 'repo')
cocoaVersion=$(getProperty 'version')

cocoaFrameworkUrl="${cocoaRepo}/releases/download/${cocoaVersion}/Sentry-Dynamic.xcframework.zip"

if ! [ -d "$sentryCocoaCache" ]; then
    mkdir $sentryCocoaCache
fi

curl -L "${cocoaFrameworkUrl}" -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip"

unzip -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip" -d "${sentryCocoaCache}/"

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"

cp "${sentryCocoaCache}/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Sentry" "${sentryArtifactsDestination}/bin/sentry.dylib"

cp -rL "${sentryCocoaCache}/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Headers" "${sentryArtifactsDestination}/include/Sentry"
cp -rL "${sentryCocoaCache}/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/PrivateHeaders/." "${sentryArtifactsDestination}/include/Sentry"