#!/bin/bash
set -euo pipefail

export sentryJavaRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

pushd ${sentryJavaRoot}
./gradlew -PsentryAndroidSdkName=sentry.native.android.unreal \
    :sentry-android-core:assembleRelease :sentry-android-ndk:assembleRelease :sentry:jar \
    --no-daemon --stacktrace --warning-mode none
popd

cp "${sentryJavaRoot}/sentry-android-ndk/build/outputs/aar/sentry-android-ndk-release.aar" "${sentryArtifactsDestination}/sentry-android-ndk-release.aar"
cp "${sentryJavaRoot}/sentry-android-core/build/outputs/aar/sentry-android-core-release.aar" "${sentryArtifactsDestination}/sentry-android-core-release.aar"

cp "${sentryJavaRoot}/sentry/build/libs/"sentry-*.jar "${sentryArtifactsDestination}/sentry.jar"

# With version v8 of the sentry-java the Native SDK NDK has to be downloaded separately from the sentry-native repo release page
configFile="${sentryJavaRoot}/gradle/libs.versions.toml"
if [[ ! -f "$configFile" ]]; then
  echo "Error: libs.versions.toml file not found at $configFile"
  exit 1
fi

nativeNdkVersion=$(grep 'sentry-native-ndk' "$configFile" | sed -E 's/.*version\s*=\s*"([^"]+)".*/\1/')

if [[ -z "$nativeNdkVersion" ]]; then
  echo "Error: Failed to extract Native SDK NDK version."
  exit 1
fi

echo "Extracted Sentry Native NDK version: $nativeNdkVersion"

nativeNdkCache="${sentryJavaRoot}/native-ndk-cache"
if ! [ -d "$nativeNdkCache" ]; then
    mkdir "$nativeNdkCache"
fi

nativeNdkUrl="https://github.com/getsentry/sentry-native/releases/download/${nativeNdkVersion}/sentry-native-ndk-${nativeNdkVersion}.zip"

curl -L "${nativeNdkUrl}" -o "${nativeNdkCache}/sentry-native-ndk-${nativeNdkVersion}.zip"

unzip -o "${nativeNdkCache}/sentry-native-ndk-${nativeNdkVersion}.zip" -d "${nativeNdkCache}/"

cp "${nativeNdkCache}/sentry-native-ndk-${nativeNdkVersion}/sentry-native-ndk-release.aar" "${sentryArtifactsDestination}/sentry-native-ndk-release.aar"