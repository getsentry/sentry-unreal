#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

# Build sentry-native using clang and libc++ for static libs
cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF \
    -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_C_COMPILER=clang-13 -D CMAKE_CXX_COMPILER="clang++-13" \
    -D CMAKE_C_FLAGS="-mno-outline-atomics" -D CMAKE_CXX_FLAGS="-stdlib=libc++ -mno-outline-atomics" -D CMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -mno-outline-atomics"
cmake --build "${sentryNativeRoot}/build" --target sentry --parallel
cmake --install "${sentryNativeRoot}/build" --prefix "${sentryNativeRoot}/install"

# Build sentry-native using clang and libstdc++ for crashpad executable
cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build_crashpad_handler" -D SENTRY_BACKEND=crashpad -D SENTRY_TRANSPORT=none -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF \
    -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_C_COMPILER=clang-13 -D CMAKE_CXX_COMPILER="clang++-13" \
    -D CMAKE_CXX_FLAGS="-stdlib=libstdc++" -D CMAKE_EXE_LINKER_FLAGS="-stdlib=libstdc++"
cmake --build "${sentryNativeRoot}/build_crashpad_handler" --target sentry --parallel
cmake --install "${sentryNativeRoot}/build_crashpad_handler" --prefix "${sentryNativeRoot}/install_crashpad_handler"

# Replace crashpad_handler obtained after the first build with the matching binary from the second one
cp "${sentryNativeRoot}/install_crashpad_handler/bin/crashpad_handler" "${sentryNativeRoot}/install/bin/crashpad_handler"

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp -r "${sentryNativeRoot}/install/lib/"*.a "${sentryArtifactsDestination}/lib"
strip -x "${sentryNativeRoot}/install/bin/crashpad_handler" -o "${sentryArtifactsDestination}/bin/crashpad_handler"
cp "${sentryNativeRoot}/install/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"
