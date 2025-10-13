#!/bin/bash

TARGET_PLATFORM="$1"
TARGET_NAME="$2"
TARGET_TYPE="$3"
TARGET_CONFIGURATION="$4"
PROJECT_FILE="$5"
PLUGIN_DIR="$6"
ENGINE_DIR="$7"

# Copy sentry.dylib to plugin's Binaries/Mac dir if it doesn't exist there to ensure FAB version of the plugin works correctly
# Epic obfuscates any extra binaries when pre-building the plugin
if [ "$TARGET_PLATFORM" = "Mac" ] && [ ! -f "$PLUGIN_DIR/Binaries/Mac/sentry.dylib" ]; then
  cp "$PLUGIN_DIR/Source/ThirdParty/Mac/bin/sentry.dylib" "$PLUGIN_DIR/Binaries/Mac/sentry.dylib"
fi

# Grant execute permissions to sentry-cli binary (FAB version of the plugin doesn't preserve file permissions)
if [ -f "$PLUGIN_DIR/Source/ThirdParty/CLI/sentry-cli-Darwin-universal" ]; then
  chmod +x "$PLUGIN_DIR/Source/ThirdParty/CLI/sentry-cli-Darwin-universal"
fi

# Call Python script for debug symbol upload
"$ENGINE_DIR/Binaries/ThirdParty/Python3/Mac/bin/python3" "$PLUGIN_DIR/Scripts/upload-debug-symbols.py" \
  "$TARGET_PLATFORM" "$TARGET_NAME" "$TARGET_TYPE" "$TARGET_CONFIGURATION" "$PROJECT_FILE" "$PLUGIN_DIR"
