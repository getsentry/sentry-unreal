#!/bin/bash
export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal
cmake --build "${sentryNativeRoot}/build" --target sentry --config RelWithDebInfo --parallel
cmake --build "${sentryNativeRoot}/build" --target crashpad_handler --config Release --parallel

ls "${sentryNativeRoot}/build"

cp "${sentryNativeRoot}/build/RelWithDebInfo/sentry.lib" "${sentryArtifactsDestination}/lib/sentry.lib"
cp "${sentryNativeRoot}/build/RelWithDebInfo/sentry.dll" "${sentryArtifactsDestination}/bin/sentry.dll"
cp "${sentryNativeRoot}/build/RelWithDebInfo/sentry.pdb" "${sentryArtifactsDestination}/bin/sentry.pdb"
cp "${sentryNativeRoot}/build/crashpad_build/handler/Release/crashpad_handler.exe" "${sentryArtifactsDestination}/bin/crashpad_handler.exe"
cp "${sentryNativeRoot}/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"