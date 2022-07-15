#!/bin/bash
export sentryIosRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

carthage build --project-directory "${sentryIosRoot}" --use-xcframeworks --no-skip-current --platform iOS

cp -R "${sentryIosRoot}/Carthage/Build/Sentry.xcframework/ios-arm64_armv7/Sentry.framework" "${sentryArtifactsDestination}/Sentry.framework"

mkdir "${sentryArtifactsDestination}/Sentry.embeddedframework"
cp -R "${sentryArtifactsDestination}/Sentry.framework" "${sentryArtifactsDestination}/Sentry.embeddedframework"
zip -r "${sentryArtifactsDestination}/Sentry.embeddedframework.zip" "${sentryArtifactsDestination}/Sentry.embeddedframework"
rm -rf "${sentryArtifactsDestination}/Sentry.embeddedframework"