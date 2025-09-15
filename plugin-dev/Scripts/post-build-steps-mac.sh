#!/bin/bash

TARGET_PLATFORM="$1"
TARGET_NAME="$2"
TARGET_TYPE="$3"
TARGET_CONFIGURATION="$4"
PROJECT_FILE="$5"
PLUGIN_DIR="$6"
ENGINE_DIR="$7"

# Copy sentry.dylib to plugin's Binaries\Mac dir if it doesn't exist there to ensure FAB version of the plugin works correctly - Epic obfuscates any extra binaries when pre-building the plugin
if [ "$TARGET_PLATFORM" = "Mac" ] && [ ! -f "$PLUGIN_DIR/Binaries/Mac/sentry.dylib" ]; then
  cp "$PLUGIN_DIR/Source/ThirdParty/Mac/bin/sentry.dylib" "$PLUGIN_DIR/Binaries/Mac/sentry.dylib"
fi

# Grant execute permissions to sentry-cli binary (FAB version of the plugin doesn't preserve file permissions)
if [ -f "$PLUGIN_DIR/Source/ThirdParty/CLI/sentry-cli-Darwin-universal" ]; then
  chmod +x "$PLUGIN_DIR/Source/ThirdParty/CLI/sentry-cli-Darwin-universal"
fi

ENGINE_VERSION=$(grep -o '"EngineAssociation": *"[^"]*"' "$PROJECT_FILE" | cut -d'"' -f4)

if [[ "${ENGINE_VERSION:0:1}" == "4" ]]; then
  EDITOR_EXE="$ENGINE_DIR/Binaries/Mac/UE4Editor"
else
  EDITOR_EXE="$ENGINE_DIR/Binaries/Mac/UnrealEditor"
fi

"$EDITOR_EXE" "$PROJECT_FILE" -run=SentrySymbolUpload -target-platform="$TARGET_PLATFORM" -target-name="$TARGET_NAME" -target-type="$TARGET_TYPE" -target-configuration="$TARGET_CONFIGURATION" -unattended -nopause -nullrhi