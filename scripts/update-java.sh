#!/usr/bin/env bash
set -euo pipefail

submodulePath="modules/sentry-java"

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
    echo "Setting submodule version to '$version'"
    git -C $submodulePath checkout $version
    ./scripts/bump-version-android.sh $version
    ;;
*)
    echo "Unknown argument $1"
    exit 1
    ;;
esac
