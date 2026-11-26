#!/bin/bash
set -euo pipefail

export sentryCocoaRoot=$1
export sentryArtifactsDestination=$2

mkdir -p "${sentryArtifactsDestination}"
rm -rf "${sentryArtifactsDestination}/"*

sentryArtifactsDestination=$(cd "${sentryArtifactsDestination}" && pwd)

cocoaDynamicFramework="${sentryCocoaRoot}/SentryObjC-Dynamic.xcframework"

# Build SentryObjC as a dynamic framework for iOS devices and simulators
rm -rf "${cocoaDynamicFramework}"
make -C "${sentryCocoaRoot}" build-xcframework-sentryobjc-dynamic SDKS=iphoneos,iphonesimulator

stagingDir=$(mktemp -d)
trap 'rm -rf "${stagingDir}"' EXIT

# Embedded framework with the device slice is used by engine versions that can't consume zipped XCFrameworks (UE < 5.5)
mkdir "${stagingDir}/SentryObjC.embeddedframework"

cp -R "${cocoaDynamicFramework}/ios-arm64/SentryObjC.framework" "${stagingDir}/SentryObjC.embeddedframework"

# XCFramework with the device and simulator slices is used by UE 5.5+
mkdir -p "${stagingDir}/SentryObjC.xcframework/ios-arm64" "${stagingDir}/SentryObjC.xcframework/ios-arm64_x86_64-simulator"

cp -R "${cocoaDynamicFramework}/ios-arm64/SentryObjC.framework" "${stagingDir}/SentryObjC.xcframework/ios-arm64"
cp -R "${cocoaDynamicFramework}/ios-arm64_x86_64-simulator/SentryObjC.framework" "${stagingDir}/SentryObjC.xcframework/ios-arm64_x86_64-simulator"
cp "${cocoaDynamicFramework}/Info.plist" "${stagingDir}/SentryObjC.xcframework/Info.plist"

# Keep the manifest in sync with the copied slices
xcframeworkManifest="${stagingDir}/SentryObjC.xcframework/Info.plist"
libraryCount=$(plutil -extract AvailableLibraries raw "$xcframeworkManifest")

for (( i = libraryCount - 1; i >= 0; i-- )); do
    libraryIdentifier=$(plutil -extract "AvailableLibraries.$i.LibraryIdentifier" raw "$xcframeworkManifest")

    case "$libraryIdentifier" in
        ios-arm64 | ios-arm64_x86_64-simulator) plutil -remove "AvailableLibraries.$i.DebugSymbolsPath" "$xcframeworkManifest" || true ;;
        *) plutil -remove "AvailableLibraries.$i" "$xcframeworkManifest" ;;
    esac
done

pushd "${stagingDir}"

zip -r "${sentryArtifactsDestination}/SentryObjC.embeddedframework.zip" "SentryObjC.embeddedframework"
zip -r "${sentryArtifactsDestination}/SentryObjC.xcframework.zip" "SentryObjC.xcframework"

popd
