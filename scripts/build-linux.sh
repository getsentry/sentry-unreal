#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=crashpad -D SENTRY_TRANSPORT=none -D SENTRY_SDK_NAME=sentry.native.unreal -D CMAKE_BUILD_TYPE=RelWithDebInfo -D SENTRY_BUILD_SHARED_LIBS=OFF
cmake --build "${sentryNativeRoot}/build" --target sentry --parallel
cmake --install "${sentryNativeRoot}/build" --prefix "${sentryNativeRoot}/install"

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

ls "${sentryNativeRoot}"

cp -r "${sentryNativeRoot}/install/lib/*.a" "${sentryArtifactsDestination}/lib"
strip -x "${sentryNativeRoot}/install/bin/crashpad_handler" -o "${sentryArtifactsDestination}/bin/crashpad_handler"
cp "${sentryNativeRoot}/install/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"
