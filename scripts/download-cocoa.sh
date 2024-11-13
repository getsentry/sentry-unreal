#!/bin/bash
set -euo pipefail

function getProperty {
    cat "$(dirname $0)/../modules/sentry-cocoa.properties" | grep $1 | cut -d'=' -f2 | tr -d '"'
}

export sentryCocoaCache=$1
export sentryArtifactsDestination=$2

if ! [ -d "$sentryCocoaCache" ]; then
    mkdir $sentryCocoaCache
fi

cocoaRepo=$(getProperty 'repo')
cocoaVersion=$(getProperty 'version')

cocoaFrameworkUrl="${cocoaRepo}/releases/download/${cocoaVersion}/Sentry-Dynamic.xcframework.zip"

curl -L "${cocoaFrameworkUrl}" -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip"

unzip -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip" -d "${sentryCocoaCache}/"

# Prepare iOS artifacts
rm -rf "${sentryArtifactsDestination}/../IOS/"*

cp -R "${sentryCocoaCache}/Sentry-Dynamic.xcframework/ios-arm64_arm64e/Sentry.framework" "${sentryArtifactsDestination}/../IOS/Sentry.framework"

mkdir "Sentry.embeddedframework"

cp -R "${sentryArtifactsDestination}/../IOS/Sentry.framework" "Sentry.embeddedframework"
zip -r "Sentry.embeddedframework.zip" "Sentry.embeddedframework"
cp -R "Sentry.embeddedframework.zip" "${sentryArtifactsDestination}/../IOS/Sentry.embeddedframework.zip"

rm -rf "Sentry.embeddedframework"
rm "Sentry.embeddedframework.zip"

# Prepare Mac artifacts
rm -rf "${sentryArtifactsDestination}/../Mac/"*

mkdir "${sentryArtifactsDestination}/../Mac/bin"
mkdir "${sentryArtifactsDestination}/../Mac/include"

cp "${sentryCocoaCache}/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Sentry" "${sentryArtifactsDestination}/../Mac/bin/sentry.dylib"

cp -rL "${sentryCocoaCache}/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Headers" "${sentryArtifactsDestination}/../Mac/include/Sentry"
cp -rL "${sentryCocoaCache}/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/PrivateHeaders/." "${sentryArtifactsDestination}/../Mac/include/Sentry"