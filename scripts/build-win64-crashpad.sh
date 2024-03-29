#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

cmake -G "Visual Studio 16 2019" -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF -D CRASHPAD_ENABLE_STACKTRACE=ON
cmake --build "${sentryNativeRoot}/build" --target sentry --config RelWithDebInfo --parallel
cmake --build "${sentryNativeRoot}/build" --target crashpad_handler --config RelWithDebInfo --parallel
cmake --install "${sentryNativeRoot}/build" --prefix "${sentryNativeRoot}/install" --config RelWithDebInfo

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp ${sentryNativeRoot}/install/lib/*.lib ${sentryArtifactsDestination}/lib
cp ${sentryNativeRoot}/install/bin/crashpad_handler.exe ${sentryArtifactsDestination}/bin/crashpad_handler.exe
cp ${sentryNativeRoot}/install/include/sentry.h ${sentryArtifactsDestination}/include/sentry.h
