#!/bin/bash
set -euo pipefail

export sentryCocoaRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

carthage build --project-directory "${sentryCocoaRoot}" --no-skip-current --platform macOS

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"

cp "${sentryCocoaRoot}/Carthage/Build/Mac/Sentry.framework/Sentry" "${sentryArtifactsDestination}/bin/sentry.dylib"

cp -R "${sentryCocoaRoot}/Carthage/Build/Mac/Sentry.framework/Headers" "${sentryArtifactsDestination}/include/Headers"
cp -R "${sentryCocoaRoot}/Carthage/Build/Mac/Sentry.framework/PrivateHeaders" "${sentryArtifactsDestination}/include/PrivateHeaders"