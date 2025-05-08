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

# Prepare iOS artifacts

cocoaDynamicFrameworkUrl="${cocoaRepo}/releases/download/${cocoaVersion}/Sentry-Dynamic.xcframework.zip"

curl -L "${cocoaDynamicFrameworkUrl}" -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip"

unzip -o "${sentryCocoaCache}/Sentry-Dynamic.xcframework.zip" -d "${sentryCocoaCache}/"

if ! [ -d "$(dirname $sentryArtifactsDestination)/IOS" ]; then
    mkdir "$(dirname $sentryArtifactsDestination)/IOS"
else
    rm -rf "$(dirname $sentryArtifactsDestination)/IOS/"*
fi

cp -R "${sentryCocoaCache}/Sentry-Dynamic.xcframework/ios-arm64_arm64e/Sentry.framework" "$(dirname $sentryArtifactsDestination)/IOS/Sentry.framework"

mkdir "Sentry.embeddedframework"

cp -R "$(dirname $sentryArtifactsDestination)/IOS/Sentry.framework" "Sentry.embeddedframework"
zip -r "Sentry.embeddedframework.zip" "Sentry.embeddedframework"
cp -R "Sentry.embeddedframework.zip" "$(dirname $sentryArtifactsDestination)/IOS/Sentry.embeddedframework.zip"

rm -rf "Sentry.embeddedframework"
rm "Sentry.embeddedframework.zip"

# Prepare Mac artifacts

cocoaStaticFrameworkUrl="${cocoaRepo}/releases/download/${cocoaVersion}/Sentry.xcframework.zip"

curl -L "${cocoaStaticFrameworkUrl}" -o "${sentryCocoaCache}/Sentry.xcframework.zip"

unzip -o "${sentryCocoaCache}/Sentry.xcframework.zip" -d "${sentryCocoaCache}/"

if ! [ -d "$(dirname $sentryArtifactsDestination)/Mac" ]; then
    mkdir "$(dirname $sentryArtifactsDestination)/Mac"
else
    rm -rf "$(dirname $sentryArtifactsDestination)/Mac/"*
fi

mkdir "$(dirname $sentryArtifactsDestination)/Mac/lib"
mkdir "$(dirname $sentryArtifactsDestination)/Mac/include"

cp "${sentryCocoaCache}/Sentry.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Sentry" "$(dirname $sentryArtifactsDestination)/Mac/lib/libsentry.a"

cp -rL "${sentryCocoaCache}/Sentry.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/Headers" "$(dirname $sentryArtifactsDestination)/Mac/include/Sentry"
cp -rL "${sentryCocoaCache}/Sentry.xcframework/macos-arm64_arm64e_x86_64/Sentry.framework/PrivateHeaders/." "$(dirname $sentryArtifactsDestination)/Mac/include/Sentry"