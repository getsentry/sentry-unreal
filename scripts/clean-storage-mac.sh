#!/bin/sh
# Frees disk space on GitHub-hosted macOS runners, which guarantee only 14 GB of
# free storage - not enough for an Unreal Engine install plus a cooked build.
# Keeps the Xcode at $XCODE_APP and removes every other Xcode and the Android SDK.
set -eu

: "${XCODE_APP:?}"

df -h /

for app in /Applications/Xcode_*.app; do
  # Version aliases such as Xcode_26.6.0.app are symlinks to the real bundles
  if [ -L "$app" ] || [ "$app" = "$XCODE_APP" ]; then
    continue
  fi
  sudo rm -rf "$app"
done

sudo rm -rf "$HOME/Library/Android"

df -h /
