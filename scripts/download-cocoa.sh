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

# TEMP PATCH: pull SentryObjC from sentry-cocoa CI artifact for Metrics API evaluation
gh run download 24890645262 -R getsentry/sentry-cocoa \
    -n "xcframework-ea03ca4e38bb6a0986deec9b16ae62b5b3db0fac-sentryobjc-dynamic" \
    -D "${sentryCocoaCache}"

unzip -o "${sentryCocoaCache}/SentryObjC-Dynamic.xcframework.zip" -d "${sentryCocoaCache}/"

# Prepare iOS artifacts
if ! [ -d "$(dirname $sentryArtifactsDestination)/IOS" ]; then
    mkdir "$(dirname $sentryArtifactsDestination)/IOS"
else
    rm -rf "$(dirname $sentryArtifactsDestination)/IOS/"*
fi

cp -R "${sentryCocoaCache}/SentryObjC-Dynamic.xcframework/ios-arm64/SentryObjC.framework" "$(dirname $sentryArtifactsDestination)/IOS/SentryObjC.framework"

mkdir "SentryObjC.embeddedframework"

cp -R "$(dirname $sentryArtifactsDestination)/IOS/SentryObjC.framework" "SentryObjC.embeddedframework"
zip -r "SentryObjC.embeddedframework.zip" "SentryObjC.embeddedframework"
cp -R "SentryObjC.embeddedframework.zip" "$(dirname $sentryArtifactsDestination)/IOS/SentryObjC.embeddedframework.zip"

rm -rf "SentryObjC.embeddedframework"
rm "SentryObjC.embeddedframework.zip"

# Prepare Mac artifacts
if ! [ -d "$(dirname $sentryArtifactsDestination)/Mac/Cocoa" ]; then
    mkdir -p "$(dirname $sentryArtifactsDestination)/Mac/Cocoa"
else
    rm -rf "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/"*
fi

mkdir "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/bin"
mkdir "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/include"

cp "${sentryCocoaCache}/SentryObjC-Dynamic.xcframework/macos-arm64_x86_64/SentryObjC.framework/SentryObjC" "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/bin/SentryObjC.dylib"

cp -rL "${sentryCocoaCache}/SentryObjC-Dynamic.xcframework/macos-arm64_x86_64/SentryObjC.framework/Headers" "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/include/SentryObjC"
