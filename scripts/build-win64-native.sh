#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

# Force CMake to use the VS2019 toolset (`-T v142`) to maintain native libraries compatibility with older Unreal Engine versions (4.27–5.1)
cmake -G "Visual Studio 17 2022" -T v142 -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build_native" -D SENTRY_BACKEND=native -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF
cmake --build "${sentryNativeRoot}/build_native" --target sentry --config RelWithDebInfo --parallel
cmake --build "${sentryNativeRoot}/build_native" --target sentry-crash --config RelWithDebInfo --parallel
cmake --install "${sentryNativeRoot}/build_native" --prefix "${sentryNativeRoot}/install_native" --config RelWithDebInfo

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp ${sentryNativeRoot}/install_native/lib/*.lib ${sentryArtifactsDestination}/lib
cp ${sentryNativeRoot}/install_native/bin/sentry-crash.exe ${sentryArtifactsDestination}/bin/sentry-crash.exe
cp ${sentryNativeRoot}/install_native/include/sentry.h ${sentryArtifactsDestination}/include/sentry.h
