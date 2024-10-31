#!/bin/bash
set -euo pipefail

export sentryIosRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

pushd "${sentryIosRoot}"

xcodebuild archive \
    -project Sentry.xcodeproj/ \
    -scheme "Sentry" \
    -configuration "Release" \
    -sdk "iphoneos" \
    -archivePath "./Carthage/archive/Sentry/iphoneos.xcarchive" \
    CODE_SIGNING_REQUIRED=NO \
    SKIP_INSTALL=NO \
    CODE_SIGN_IDENTITY= \
    CARTHAGE=YES \
    MACH_O_TYPE="mh_dylib" \
    ENABLE_CODE_COVERAGE=NO \
    GCC_GENERATE_DEBUGGING_SYMBOLS="YES" \
    OTHER_LDFLAGS="-Wl,-make_mergeable"

xcodebuild -create-xcframework -framework Carthage/archive/Sentry/iphoneos.xcarchive/Products/Library/Frameworks/Sentry.framework -output Carthage/Sentry.xcframework

popd

cp -R "${sentryIosRoot}/Carthage/Sentry.xcframework/ios-arm64_arm64e/Sentry.framework" "${sentryArtifactsDestination}/Sentry.framework"

mkdir "Sentry.embeddedframework"

cp -R "${sentryArtifactsDestination}/Sentry.framework" "Sentry.embeddedframework"
zip -r "Sentry.embeddedframework.zip" "Sentry.embeddedframework"
cp -R "Sentry.embeddedframework.zip" "${sentryArtifactsDestination}/Sentry.embeddedframework.zip"

rm -rf "Sentry.embeddedframework"
rm "Sentry.embeddedframework.zip"