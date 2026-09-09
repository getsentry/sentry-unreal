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
cocoaDynamicFramework="SentryObjC-Dynamic.xcframework"
cocoaDynamicFrameworkZip="${cocoaDynamicFramework}.zip"
cocoaDynamicFrameworkUrl="${cocoaRepo}/releases/download/${cocoaVersion}/${cocoaDynamicFrameworkZip}"

curl -L "${cocoaDynamicFrameworkUrl}" -o "${sentryCocoaCache}/${cocoaDynamicFrameworkZip}"

unzip -o "${sentryCocoaCache}/${cocoaDynamicFrameworkZip}" -d "${sentryCocoaCache}/"

# Prepare iOS artifacts
if ! [ -d "$(dirname $sentryArtifactsDestination)/IOS" ]; then
    mkdir "$(dirname $sentryArtifactsDestination)/IOS"
else
    rm -rf "$(dirname $sentryArtifactsDestination)/IOS/"*
fi

cp -R "${sentryCocoaCache}/${cocoaDynamicFramework}/ios-arm64/SentryObjC.framework" "$(dirname $sentryArtifactsDestination)/IOS/SentryObjC.framework"

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

cp "${sentryCocoaCache}/${cocoaDynamicFramework}/macos-arm64_x86_64/SentryObjC.framework/SentryObjC" "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/bin/SentryObjC.dylib"

cp -rL "${sentryCocoaCache}/${cocoaDynamicFramework}/macos-arm64_x86_64/SentryObjC.framework/Headers" "$(dirname $sentryArtifactsDestination)/Mac/Cocoa/include/SentryObjC"
