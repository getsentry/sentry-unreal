#!/bin/bash
set -euo pipefail

export sentryIosRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

carthage build --project-directory "${sentryIosRoot}" --use-xcframeworks --no-skip-current --platform iOS

cp -R "${sentryIosRoot}/Carthage/Build/Sentry.xcframework/ios-arm64_arm64e/Sentry.framework" "${sentryArtifactsDestination}/Sentry.framework"

mkdir "Sentry.embeddedframework"

cp -R "${sentryArtifactsDestination}/Sentry.framework" "Sentry.embeddedframework"
zip -r "Sentry.embeddedframework.zip" "Sentry.embeddedframework"
cp -R "Sentry.embeddedframework.zip" "${sentryArtifactsDestination}/Sentry.embeddedframework.zip"

rm -rf "Sentry.embeddedframework"
rm "Sentry.embeddedframework.zip"