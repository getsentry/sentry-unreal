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

cocoaRepo=$(getProperty 'repo')
cocoaVersion=$(getProperty 'version')

cocoaDynamicFrameworkUrl="${cocoaRepo}/releases/download/${cocoaVersion}/Sentry-Dynamic.xcframework.zip"

curl -L "${cocoaDynamicFrameworkUrl}" -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip"

unzip -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip" -d "${sentryCocoaCache}/"

# Prepare iOS artifacts
if ! [ -d "$(dirname $sentryArtifactsDestination)/IOS" ]; then
    mkdir "$(dirname $sentryArtifactsDestination)/IOS"
else
    rm -rf "$(dirname $sentryArtifactsDestination)/IOS/"*
fi

cp -R "${sentryCocoaCache}/Sentry-Dynamic.xcframework/ios-arm64/Sentry.framework" "$(dirname $sentryArtifactsDestination)/IOS/Sentry.framework"

mkdir "Sentry.embeddedframework"

cp -R "$(dirname $sentryArtifactsDestination)/IOS/Sentry.framework" "Sentry.embeddedframework"
zip -r "Sentry.embeddedframework.zip" "Sentry.embeddedframework"
cp -R "Sentry.embeddedframework.zip" "$(dirname $sentryArtifactsDestination)/IOS/Sentry.embeddedframework.zip"

rm -rf "Sentry.embeddedframework"
rm "Sentry.embeddedframework.zip"

# Prepare Mac artifacts
if ! [ -d "$(dirname $sentryArtifactsDestination)/Mac/Cocoa" ]; then
    mkdir -p "$(dirname $sentryArtifactsDestination)/Mac/Cocoa"
else
    rm -rf "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/"*
fi

mkdir "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/bin"
mkdir "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/include"

cp "${sentryCocoaCache}/Sentry-Dynamic.xcframework/macos-arm64_x86_64/Sentry.framework/Sentry" "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/bin/sentry.dylib"

cp -rL "${sentryCocoaCache}/Sentry-Dynamic.xcframework/macos-arm64_x86_64/Sentry.framework/Headers" "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/include/Sentry"
cp -rL "${sentryCocoaCache}/Sentry-Dynamic.xcframework/macos-arm64_x86_64/Sentry.framework/PrivateHeaders/." "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/include/Sentry"