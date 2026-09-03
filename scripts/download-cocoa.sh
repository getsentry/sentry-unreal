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

# Embedded framework with the device slice is used by engine versions that can't consume zipped XCFrameworks (UE < 5.5)
mkdir "SentryObjC.embeddedframework"

cp -R "${sentryCocoaCache}/${cocoaDynamicFramework}/ios-arm64/SentryObjC.framework" "SentryObjC.embeddedframework"
zip -r "SentryObjC.embeddedframework.zip" "SentryObjC.embeddedframework"
cp -R "SentryObjC.embeddedframework.zip" "$(dirname $sentryArtifactsDestination)/IOS/SentryObjC.embeddedframework.zip"

rm -rf "SentryObjC.embeddedframework"
rm "SentryObjC.embeddedframework.zip"

# XCFramework with the device and simulator slices is used by UE 5.5+
mkdir -p "SentryObjC.xcframework/ios-arm64" "SentryObjC.xcframework/ios-arm64_x86_64-simulator"

cp -R "${sentryCocoaCache}/${cocoaDynamicFramework}/ios-arm64/SentryObjC.framework" "SentryObjC.xcframework/ios-arm64"
cp -R "${sentryCocoaCache}/${cocoaDynamicFramework}/ios-arm64_x86_64-simulator/SentryObjC.framework" "SentryObjC.xcframework/ios-arm64_x86_64-simulator"
cp "${sentryCocoaCache}/${cocoaDynamicFramework}/Info.plist" "SentryObjC.xcframework/Info.plist"

# Keep the manifest in sync with the copied slices
xcframeworkManifest="SentryObjC.xcframework/Info.plist"
libraryCount=$(plutil -extract AvailableLibraries raw "$xcframeworkManifest")

for (( i = libraryCount - 1; i >= 0; i-- )); do
    libraryIdentifier=$(plutil -extract "AvailableLibraries.$i.LibraryIdentifier" raw "$xcframeworkManifest")

    case "$libraryIdentifier" in
        ios-arm64 | ios-arm64_x86_64-simulator) plutil -remove "AvailableLibraries.$i.DebugSymbolsPath" "$xcframeworkManifest" || true ;;
        *) plutil -remove "AvailableLibraries.$i" "$xcframeworkManifest" ;;
    esac
done

zip -r "SentryObjC.xcframework.zip" "SentryObjC.xcframework"
cp -R "SentryObjC.xcframework.zip" "$(dirname $sentryArtifactsDestination)/IOS/SentryObjC.xcframework.zip"

rm -rf "SentryObjC.xcframework"
rm "SentryObjC.xcframework.zip"

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
