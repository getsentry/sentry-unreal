#!/usr/bin/env bash
set -euo pipefail

# Usage: build-crash-reporter.sh <submodule-path> <output-path> <runtime-id>
# Example: build-crash-reporter.sh modules/sentry-crash-reporter \
#   plugin-dev/Source/ThirdParty/Win64/bin win-x64

SUBMODULE_PATH="$1"
OUTPUT_PATH="$2"
RUNTIME_ID="$3"

TEMP_DIR=$(mktemp -d)

dotnet publish \
    -f net9.0-desktop \
    -r "$RUNTIME_ID" \
    "$SUBMODULE_PATH/Sentry.CrashReporter/Sentry.CrashReporter.csproj" \
    -o "$TEMP_DIR"

mkdir -p "$OUTPUT_PATH"

if [[ "$RUNTIME_ID" == win-* ]]; then
    mv "$TEMP_DIR/Sentry.CrashReporter.exe" "$OUTPUT_PATH/Sentry.CrashReporter.exe"
else
    mv "$TEMP_DIR/Sentry.CrashReporter" "$OUTPUT_PATH/Sentry.CrashReporter"
fi

rm -rf "$TEMP_DIR"
