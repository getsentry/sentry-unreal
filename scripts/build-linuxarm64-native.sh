#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*

# Build sentry-native using clang and libc++ for static libs
cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build_native" -D SENTRY_BACKEND=native -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF \
    -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_C_COMPILER=clang-13 -D CMAKE_CXX_COMPILER="clang++-13" \
    -D CMAKE_C_FLAGS="-mno-outline-atomics" -D CMAKE_CXX_FLAGS="-stdlib=libc++ -mno-outline-atomics" -D CMAKE_EXE_LINKER_FLAGS="-stdlib=libc++ -mno-outline-atomics" \
    -D HAVE_COPY_FILE_RANGE=0
cmake --build "${sentryNativeRoot}/build_native" --target sentry --parallel
cmake --install "${sentryNativeRoot}/build_native" --prefix "${sentryNativeRoot}/install_native"

# Build sentry-native using clang and libstdc++ for sentry-crash executable
# Unlike crashpad_handler, sentry-crash needs transport (curl) to send crash envelopes
cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build_native_crash" -D SENTRY_BACKEND=native -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF \
    -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_C_COMPILER=clang-13 -D CMAKE_CXX_COMPILER="clang++-13" \
    -D CMAKE_CXX_FLAGS="-stdlib=libstdc++" -D CMAKE_EXE_LINKER_FLAGS="-stdlib=libstdc++" \
    -D HAVE_COPY_FILE_RANGE=0
cmake --build "${sentryNativeRoot}/build_native_crash" --target sentry-crash --parallel

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp -r "${sentryNativeRoot}/install_native/lib/"*.a "${sentryArtifactsDestination}/lib"
strip -x "${sentryNativeRoot}/build_native_crash/sentry-crash" -o "${sentryArtifactsDestination}/bin/sentry-crash"
cp "${sentryNativeRoot}/install_native/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"
