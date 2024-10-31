#!/bin/bash
set -euo pipefail

export sentryCocoaRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

pushd "${sentryCocoaRoot}"
./scripts/build-xcframework.sh gameOnly
popd

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"

cp "${sentryCocoaRoot}/Carthage/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Sentry" "${sentryArtifactsDestination}/bin/sentry.dylib"

cp -rL "${sentryCocoaRoot}/Carthage/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Sentry/Headers" "${sentryArtifactsDestination}/include/Sentry"
cp -rL "${sentryCocoaRoot}/Carthage/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Sentry/PrivateHeaders/." "${sentryArtifactsDestination}/include/Sentry"