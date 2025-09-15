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

ENGINE_VERSION=$(grep -o '"EngineAssociation": *"[^"]*"' "$PROJECT_FILE" | cut -d'"' -f4)

if [ "$(printf %.1s "$ENGINE_VERSION")" = "4" ]; then
  EDITOR_EXE="$ENGINE_DIR/Binaries/Linux/UE4Editor"
else
  EDITOR_EXE="$ENGINE_DIR/Binaries/Linux/UnrealEditor"
fi

"$EDITOR_EXE" "$PROJECT_FILE" -run=SentrySymbolUpload -target-platform="$TARGET_PLATFORM" -target-name="$TARGET_NAME" -target-type="$TARGET_TYPE" -target-configuration="$TARGET_CONFIGURATION" -unattended -nopause -nullrhi