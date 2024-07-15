#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

TOOLCHAIN_URL="http://cdn.unrealengine.com/Toolchain_Linux/native-linux-v22_clang-16.0.6-centos7.tar.gz"
TOOLCHAIN_ARCHIVE="sentry-toolchain"

TOOLCHAIN_DIR="sentry-toolchain-test"
mkdir $TOOLCHAIN_DIR

wget $TOOLCHAIN_URL -O ${TOOLCHAIN_ARCHIVE}.tmp
mv ${TOOLCHAIN_ARCHIVE}.tmp $TOOLCHAIN_ARCHIVE

tar -xvf $TOOLCHAIN_ARCHIVE -C "sentry-toolchain-test"

# Set environment variables for the toolchain (modify based on your needs)
export PATH=/home/runner/work/sentry-unreal/sentry-unreal/sentry-toolchain-test/v22_clang-16.0.6-centos7/aarch64-unknown-linux-gnueabi/bin:$PATH
export CXX=/home/runner/work/sentry-unreal/sentry-unreal/sentry-toolchain-test/v22_clang-16.0.6-centos7/aarch64-unknown-linux-gnueabi/bin/clang++
export CC=/home/runner/work/sentry-unreal/sentry-unreal/sentry-toolchain-test/v22_clang-16.0.6-centos7/aarch64-unknown-linux-gnueabi/bin/clang

rm -rf "${sentryArtifactsDestination}/"*

cmake -S "${sentryNativeRoot}" -B "${sentryNativeRoot}/build" -D SENTRY_BACKEND=crashpad -D SENTRY_TRANSPORT=none -D SENTRY_SDK_NAME=sentry.native.unreal -D SENTRY_BUILD_SHARED_LIBS=OFF \
    -D CMAKE_BUILD_TYPE=RelWithDebInfo -D CMAKE_C_COMPILER=$CC -D CMAKE_CXX_COMPILER=$CXX \
    -D CMAKE_C_COMPILE_OPTIONS_SYSROOT="/home/runner/work/sentry-unreal/sentry-unreal/sentry-toolchain-test/v22_clang-16.0.6-centos7/aarch64-unknown-linux-gnueabi" \
    -D CMAKE_CXX_COMPILE_OPTIONS_SYSROOT="/home/runner/work/sentry-unreal/sentry-unreal/sentry-toolchain-test/v22_clang-16.0.6-centos7/aarch64-unknown-linux-gnueabi"
cmake --build "${sentryNativeRoot}/build" --target sentry --parallel
cmake --install "${sentryNativeRoot}/build" --prefix "${sentryNativeRoot}/install"

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"
mkdir "${sentryArtifactsDestination}/lib"

cp -r "${sentryNativeRoot}/install/lib/"*.a "${sentryArtifactsDestination}/lib"
strip -x "${sentryNativeRoot}/install/bin/crashpad_handler" -o "${sentryArtifactsDestination}/bin/crashpad_handler"
cp "${sentryNativeRoot}/install/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"
