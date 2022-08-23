#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14
cmake --build "${sentryNativeRoot}/build" --target sentry --config RelWithDebInfo --parallel
cmake --build "${sentryNativeRoot}/build" --target crashpad_handler --config Release --parallel

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"

cp "${sentryNativeRoot}/build/libsentry.dylib" "${sentryArtifactsDestination}/bin/libsentry.dylib"
cp -R "${sentryNativeRoot}/build/libsentry.dylib.dSYM" "${sentryArtifactsDestination}/bin/libsentry.dylib.dSYM"
cp "${sentryNativeRoot}/build/crashpad_build/handler/crashpad_handler" "${sentryArtifactsDestination}/bin/crashpad_handler"
cp "${sentryNativeRoot}/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"
