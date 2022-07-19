#!/usr/bin/env bash

sed -i 's/io.sentry,sentry,.\..\../io.sentry,sentry,'$1'/g' Source/Sentry/Sentry_Android_UPL.xml
sed -i 's/io.sentry,sentry-android-core,.\..\../io.sentry,sentry-android-core,'$1'/g' Source/Sentry/Sentry_Android_UPL.xml
sed -i 's/io.sentry,sentry-android-ndk,.\..\../io.sentry,sentry-android-ndk,'$1'/g' Source/Sentry/Sentry_Android_UPL.xml