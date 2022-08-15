#!/usr/bin/env bash
set -euo pipefail

submodulePath=$"Modules/sentry-cocoa"
targetPlatform=$"iOS"

case $1 in
get-version)
    git submodule update --init --no-fetch --single-branch $submodulePath > /dev/null
    version=$(git -C $submodulePath describe --tags)
    echo $version
    ;;
get-repo)
    echo "https://github.com/getsentry/sentry-cocoa.git"
    ;;
set-version)
    version=$2

    git -C $submodulePath checkout $version

    ./Scripts/checkout-submodule.sh $submodulePath
    ./Scripts/build-cocoa.sh $submodulePath plugin-dev/Source/ThirdParty/$targetPlatform

    echo "Setting submodule version to '$version'"
    ;;
*)
    echo "Unknown argument $1"
    exit 1
    ;;
esac