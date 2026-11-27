#!/bin/bash
set -euo pipefail

export sentryCocoaRoot=$1
export sentryArtifactsDestination=$2

mkdir -p "${sentryArtifactsDestination}"
rm -rf "${sentryArtifactsDestination}/"*

cocoaDynamicFramework="${sentryCocoaRoot}/SentryObjC-Dynamic.xcframework"

# Build SentryObjC as a dynamic framework for macOS (universal binary: x86_64 + arm64)
rm -rf "${cocoaDynamicFramework}"
make -C "${sentryCocoaRoot}" build-xcframework-sentryobjc-dynamic SDKS=macosx

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"

cp "${cocoaDynamicFramework}/macos-arm64_x86_64/SentryObjC.framework/SentryObjC" "${sentryArtifactsDestination}/bin/SentryObjC.dylib"

cp -rL "${cocoaDynamicFramework}/macos-arm64_x86_64/SentryObjC.framework/Headers" "${sentryArtifactsDestination}/include/SentryObjC"
