#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

# Force CMake to use the VS2019 toolset (`-T v142`) to maintain native libraries compatibility with older Unreal Engine versions (4.27–5.1)
cmake -G "Visual Studio 17 2022" -T v142 -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF
cmake --build "${sentryNativeRoot}/build" --target sentry --config RelWithDebInfo --parallel
cmake --build "${sentryNativeRoot}/build" --target crashpad_handler --config RelWithDebInfo --parallel
cmake --install "${sentryNativeRoot}/build" --prefix "${sentryNativeRoot}/install" --config RelWithDebInfo

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp ${sentryNativeRoot}/install/lib/*.lib ${sentryArtifactsDestination}/lib
cp ${sentryNativeRoot}/install/bin/crashpad_handler.exe ${sentryArtifactsDestination}/bin/crashpad_handler.exe
cp ${sentryNativeRoot}/install/bin/crashpad_wer.dll ${sentryArtifactsDestination}/bin/crashpad_wer.dll
cp ${sentryNativeRoot}/install/include/sentry.h ${sentryArtifactsDestination}/include/sentry.h
