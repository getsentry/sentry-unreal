#!/usr/bin/env bash

TARGET_PLATFORM="$1"
TARGET_TYPE="$2"
TARGET_CONFIGURATION="$3"
PROJECT_FILE="$4"
PLUGIN_DIR="$5"
ENGINE_DIR="$6"

# Grant execute permissions to sentry-cli binary (FAB version of the plugin doesn't preserve file permissions)
if [ -f "$PLUGIN_DIR/Source/ThirdParty/CLI/sentry-cli-Linux-x86_64" ]; then
  chmod +x "$PLUGIN_DIR/Source/ThirdParty/CLI/sentry-cli-Linux-x86_64"
fi

# Skip commandlet execution for Editor target type
if [ "$TARGET_TYPE" = "Editor" ]; then
  echo "Skipping SentrySymbolUpload commandlet for Editor target type"
  return 0 2>/dev/null || exit 0
fi

ENGINE_VERSION=$(grep -o '"EngineAssociation": *"[^"]*"' "$PROJECT_FILE" | cut -d'"' -f4)

if [ "$(printf %.1s "$ENGINE_VERSION")" = "4" ]; then
  EDITOR_EXE="$ENGINE_DIR/Binaries/Linux/UE4Editor-Cmd"
else
  EDITOR_EXE="$ENGINE_DIR/Binaries/Linux/UnrealEditor-Cmd"
fi

echo "Before SentrySymbolUpload..."

"$EDITOR_EXE" "$PROJECT_FILE" -run=SentrySymbolUpload -target-platform="$TARGET_PLATFORM" -target-type="$TARGET_TYPE" -target-configuration="$TARGET_CONFIGURATION" -unattended -nopause

echo "After SentrySymbolUpload..."
