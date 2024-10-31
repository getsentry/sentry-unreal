#!/bin/bash
set -euo pipefail

export sentryCocoaRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

pushd "${sentryCocoaRoot}"

xcodebuild archive \
    -project Sentry.xcodeproj/ \
    -scheme "Sentry" \
    -configuration "Release" \
    -sdk "macosx" \
    -archivePath "./Carthage/archive/Sentry/macosx.xcarchive" \
    CODE_SIGNING_REQUIRED=NO \
    SKIP_INSTALL=NO \
    CODE_SIGN_IDENTITY= \
    CARTHAGE=YES \
    MACH_O_TYPE="mh_dylib" \
    ENABLE_CODE_COVERAGE=NO \
    GCC_GENERATE_DEBUGGING_SYMBOLS="YES" \
    OTHER_LDFLAGS="-Wl,-make_mergeable"

xcodebuild -create-xcframework -framework Carthage/archive/Sentry/macosx.xcarchive/Products/Library/Frameworks/Sentry.framework -output Carthage/Sentry.xcframework

popd

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"

cp "${sentryCocoaRoot}/Carthage/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Sentry" "${sentryArtifactsDestination}/bin/sentry.dylib"

cp -rL "${sentryCocoaRoot}/Carthage/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Sentry/Headers" "${sentryArtifactsDestination}/include/Sentry"
cp -rL "${sentryCocoaRoot}/Carthage/Sentry-Dynamic.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Sentry/PrivateHeaders/." "${sentryArtifactsDestination}/include/Sentry"