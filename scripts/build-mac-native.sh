#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

# Build sentry-native with native backend for macOS (universal binary: x86_64 + arm64)
cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build_native" -D SENTRY_BACKEND=native -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF \
    -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_OSX_ARCHITECTURES="x86_64;arm64" -D CMAKE_OSX_DEPLOYMENT_TARGET=13.0
cmake --build "${sentryNativeRoot}/build_native" --target sentry --parallel
cmake --build "${sentryNativeRoot}/build_native" --target sentry-crash --parallel
cmake --install "${sentryNativeRoot}/build_native" --prefix "${sentryNativeRoot}/install_native"

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp -r "${sentryNativeRoot}/install_native/lib/"*.a "${sentryArtifactsDestination}/lib"
cp "${sentryNativeRoot}/build_native/sentry-crash" "${sentryArtifactsDestination}/bin/sentry-crash"
cp "${sentryNativeRoot}/install_native/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"
