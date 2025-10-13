#!/bin/bash

TARGET_PLATFORM="$1"
TARGET_NAME="$2"
TARGET_TYPE="$3"
TARGET_CONFIGURATION="$4"
PROJECT_FILE="$5"
PLUGIN_DIR="$6"
ENGINE_DIR="$7"

# Grant execute permissions to sentry-cli binary (FAB version of the plugin doesn't preserve file permissions)
if [ -f "$PLUGIN_DIR/Source/ThirdParty/CLI/sentry-cli-Linux-x86_64" ]; then
  chmod +x "$PLUGIN_DIR/Source/ThirdParty/CLI/sentry-cli-Linux-x86_64"
fi

# Call Python script for debug symbol upload
"$ENGINE_DIR/Binaries/ThirdParty/Python3/Linux/bin/python3" "$PLUGIN_DIR/Scripts/upload-debug-symbols.py" \
  "$TARGET_PLATFORM" "$TARGET_NAME" "$TARGET_TYPE" "$TARGET_CONFIGURATION" "$PROJECT_FILE" "$PLUGIN_DIR"
