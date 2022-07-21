#!/bin/bash
set -euo pipefail

export sentryJavaRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

pushd ${sentryJavaRoot}
"./gradlew" -PsentryAndroidSdkName=sentry.native.android.unreal :sentry-android-core:assembleRelease :sentry-android-ndk:assembleRelease :sentry:jar --no-daemon --stacktrace --warning-mode none
popd

cp "${sentryJavaRoot}/sentry-android-ndk/build/outputs/aar/sentry-android-ndk-release.aar" "${sentryArtifactsDestination}/sentry-android-ndk-release.aar"
cp "${sentryJavaRoot}/sentry-android-core/build/outputs/aar/sentry-android-core-release.aar" "${sentryArtifactsDestination}/sentry-android-core-release.aar"

cp "${sentryJavaRoot}/sentry/build/libs/"sentry-*.jar "${sentryArtifactsDestination}/sentry.jar"