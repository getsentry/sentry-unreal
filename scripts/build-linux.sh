#!/bin/bash
set -euo pipefail

export sentryNativeRoot=$1
export sentryArtifactsDestination=$2

rm -rf "${sentryArtifactsDestination}/"*
rm -rf "${sentryNativeRoot}/build"

# Build for an older GLIBC so that it can run with the old GLIBC included in EpicGames' Unreal docker images.
# See discussion in https://github.com/getsentry/sentry-unreal/pull/173
docker run --rm dockcross/manylinux_2_28-x64 >./dockcross
chmod +x ./dockcross
./dockcross bash -c "yum install -y openssl-devel && cmake -S '${sentryNativeRoot}' -B '${sentryNativeRoot}/build' -D SENTRY_BACKEND=crashpad -D SENTRY_SDK_NAME=sentry.native.unreal -D CMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build '${sentryNativeRoot}/build' --target sentry --parallel"

mkdir "${sentryArtifactsDestination}/bin"
mkdir "${sentryArtifactsDestination}/include"

strip -s "${sentryNativeRoot}/build/libsentry.so" -w -K sentry_[^_]* -o "${sentryArtifactsDestination}/bin/libsentry.so"
cp "${sentryNativeRoot}/build/libsentry.so" "${sentryArtifactsDestination}/bin/libsentry.dbg.so"
strip -x "${sentryNativeRoot}/build/crashpad_build/handler/crashpad_handler" -o "${sentryArtifactsDestination}/bin/crashpad_handler"
cp "${sentryNativeRoot}/include/sentry.h" "${sentryArtifactsDestination}/include/sentry.h"

pushd ${sentryArtifactsDestination}/bin
objcopy --add-gnu-debuglink="libsentry.dbg.so" "libsentry.so"
popd
