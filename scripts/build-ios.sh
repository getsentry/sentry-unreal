#!/bin/bash
set -euo pipefail

export sentryIosRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

"${sentryIosRoot}/scripts//build-xcframework.sh" iOSOnly

cp -R "${sentryIosRoot}/Carthage/Build/Sentry-Dynamic.xcframework/ios-arm64_arm64e/Sentry.framework" "${sentryArtifactsDestination}/Sentry.framework"

mkdir "Sentry.embeddedframework"

cp -R "${sentryArtifactsDestination}/Sentry.framework" "Sentry.embeddedframework"
zip -r "Sentry.embeddedframework.zip" "Sentry.embeddedframework"
cp -R "Sentry.embeddedframework.zip" "${sentryArtifactsDestination}/Sentry.embeddedframework.zip"

rm -rf "Sentry.embeddedframework"
rm "Sentry.embeddedframework.zip"