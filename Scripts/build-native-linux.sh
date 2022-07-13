#!/bin/bash
export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=breakpad -D SENTRY_SDK_NAME=sentry.native.unreal -D CMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build "${sentryNativeRoot}/build" --target sentry --parallel

strip -s "${sentryNativeRoot}/build/libsentry.so" -w -K sentry_[^_]* -o "${sentryArtifactsDestination}/bin/libsentry.so"
cp "${sentryNativeRoot}/build/libsentry.so" "${sentryArtifactsDestination}/bin/libsentry.dbg.so"
cp "${sentryNativeRoot}/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"

objcopy --add-gnu-debuglink="${sentryArtifactsDestination}/bin/libsentry.dbg.so" "${sentryArtifactsDestination}/bin/libsentry.so"