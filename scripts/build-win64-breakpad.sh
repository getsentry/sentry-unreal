#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=breakpad -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF
cmake --build "${sentryNativeRoot}/build" --target sentry --config RelWithDebInfo --parallel
cmake --install "${sentryNativeRoot}/build" --prefix "${sentryNativeRoot}/install"

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp ${sentryNativeRoot}/install/lib/*.lib ${sentryArtifactsDestination}/lib
cp ${sentryNativeRoot}/install/include/sentry.h ${sentryArtifactsDestination}/include/sentry.h
