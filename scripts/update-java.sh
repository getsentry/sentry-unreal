#!/usr/bin/env bash
set -euo pipefail

submodulePath=$"modules/sentry-java"
targetPlatform=$"Android"

case $1 in
get-version)
    git submodule update --init --no-fetch --single-branch $submodulePath >/dev/null
    version=$(git -C $submodulePath describe --tags)
    echo $version
    ;;
get-repo)
    echo "https://github.com/getsentry/sentry-java.git"
    ;;
set-version)
    version=$2

    git -C $submodulePath checkout $version

    ./scripts/checkout-submodule.sh $submodulePath
    ./scripts/build-java.sh $submodulePath plugin-dev/Source/ThirdParty/$targetPlatform

    ./scripts/bump-version-android.sh $version

    echo "Setting submodule version to '$version'"
    ;;
*)
    echo "Unknown argument $1"
    exit 1
    ;;
esac
