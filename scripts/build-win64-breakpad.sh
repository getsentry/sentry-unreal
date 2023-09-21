#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=breakpad -D SENTRY_SDK_NAME=sentry.native.unreal
cmake --build "${sentryNativeRoot}/build" --target sentry --config RelWithDebInfo --parallel

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp ${sentryNativeRoot}/build/RelWithDebInfo/sentry.lib ${sentryArtifactsDestination}/lib/sentry.lib
cp ${sentryNativeRoot}/build/RelWithDebInfo/sentry.dll ${sentryArtifactsDestination}/bin/sentry.dll
cp ${sentryNativeRoot}/build/RelWithDebInfo/sentry.pdb ${sentryArtifactsDestination}/bin/sentry.pdb
cp ${sentryNativeRoot}/build/external/RelWithDebInfo/breakpad_client.lib ${sentryArtifactsDestination}/lib/breakpad_client.lib
cp ${sentryNativeRoot}/include/sentry.h ${sentryArtifactsDestination}/include/sentry.h
