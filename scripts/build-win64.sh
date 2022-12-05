#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal SENTRY_BUILD_SHARED_LIBS=OFF -D SENTRY_BUILD_SHARED_LIBS=OFF
cmake --build "${sentryNativeRoot}/build" --target sentry --config RelWithDebInfo
cmake --build "${sentryNativeRoot}/build" --target crashpad_handler --config Release
cmake --install "${sentryNativeRoot}/build" --prefix "${sentryNativeRoot}/install" --config RelWithDebInfo

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp -r ${sentryNativeRoot}/install/lib ${sentryArtifactsDestination}/lib
cp ${sentryNativeRoot}/build/crashpad_build/handler/Release/crashpad_handler.exe ${sentryArtifactsDestination}/bin/crashpad_handler.exe
cp ${sentryNativeRoot}/include/sentry.h ${sentryArtifactsDestination}/include/sentry.h
