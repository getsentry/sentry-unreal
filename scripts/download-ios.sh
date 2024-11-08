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

rm -rf "${sentryArtifactsDestination}/"*

cocoaRepo=$(getProperty 'repo')
cocoaVersion=$(getProperty 'version')

cocoaFrameworkUrl="${cocoaRepo}/releases/download/${cocoaVersion}/Sentry-Dynamic.xcframework.zip"

curl -L "${cocoaFrameworkUrl}" -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip"

unzip -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip" -d "${sentryCocoaCache}/"

cp -R "${sentryCocoaCache}/Sentry-Dynamic.xcframework/ios-arm64_arm64e/Sentry.framework" "${sentryArtifactsDestination}/Sentry.framework"

mkdir "Sentry.embeddedframework"

cp -R "${sentryArtifactsDestination}/Sentry.framework" "Sentry.embeddedframework"
zip -r "Sentry.embeddedframework.zip" "Sentry.embeddedframework"
cp -R "Sentry.embeddedframework.zip" "${sentryArtifactsDestination}/Sentry.embeddedframework.zip"

rm -rf "Sentry.embeddedframework"
rm "Sentry.embeddedframework.zip"