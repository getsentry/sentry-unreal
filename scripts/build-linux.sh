#!/bin/bash
set -euo pipefail
export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=crashpad -D SENTRY_TRANSPORT=none -D SENTRY_SDK_NAME=sentry.native.unreal -D CMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build "${sentryNativeRoot}/build" --target sentry --parallel

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"

strip -s "${sentryNativeRoot}/build/libsentry.so" -w -K sentry_[^_]* -o "${sentryArtifactsDestination}/bin/libsentry.so"
cp "${sentryNativeRoot}/build/libsentry.so" "${sentryArtifactsDestination}/bin/libsentry.dbg.so"
strip -x "${sentryNativeRoot}/build/crashpad_build/handler/crashpad_handler" -o "${sentryArtifactsDestination}/bin/crashpad_handler"
cp "${sentryNativeRoot}/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"

pushd ${sentryArtifactsDestination}/bin
objcopy --add-gnu-debuglink="libsentry.dbg.so" "libsentry.so"
popd