#!/usr/bin/env bash
set -euo pipefail

cd $(dirname "$0")/../
ANDROID_UPL_FILEPATH=plugin-dev/Source/Sentry/Sentry_Android_UPL.xml

case $1 in
get-version)
    perl -ne 'print "$1\n" if ( m/io\.sentry:sentry-android-gradle-plugin:([0-9.]+).*\/\/ current/ )' $ANDROID_UPL_FILEPATH
    ;;
get-repo)
    echo "https://github.com/getsentry/sentry-android-gradle-plugin.git"
    ;;
set-version)
    version=$2

    echo "Setting Android Gradle Plugin version to '$version'"

    PATTERN="io\.sentry:sentry-android-gradle-plugin:([0-9.]+).*\/\/ current"
    perl -pi -e "s/$PATTERN/io.sentry:sentry-android-gradle-plugin:$version \/\/ current/g" $ANDROID_UPL_FILEPATH
    ;;
*)
    echo "Unknown argument $1"
    exit 1
    ;;
esac
