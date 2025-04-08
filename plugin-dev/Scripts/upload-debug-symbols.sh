#!/usr/bin/env bash

targetPlatform=$1
targetName=$2
targetType=$3
targetConfig=$4
projectPath=$5
pluginPath=$6

PROJECT_BINARIES_PATH="$projectPath/Binaries/$targetPlatform"
PLUGIN_BINARIES_PATH="$pluginPath/Source/ThirdParty/$targetPlatform"
CONFIG_PATH="$projectPath/Config"
DEFAULT_ENGINE_INI="$CONFIG_PATH/DefaultEngine.ini"

echo "Sentry: Start debug symbols upload"

if [ $targetType = "Editor" ]; then
    echo "Sentry: Automatic symbols upload is not required for Editor target. Skipping..."
    exit
fi

if [ $targetPlatform = "Android" ]; then
    echo "Sentry: Debug symbols upload for Android is handled by Sentry's Gradle plugin (if enabled)"
    exit
fi

UPLOAD_SYMBOLS=$(awk -F "=" '/UploadSymbolsAutomatically/ {print $2}' "$DEFAULT_ENGINE_INI")

if [ ! -z $SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY ]; then
    UPLOAD_SYMBOLS=$SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY
    echo "Sentry: Automatic symbols upload settings were overridden via environment variable SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY with value '$SENTRY_UPLOAD_SYMBOLS_AUTOMATICALLY'"
fi

if [ -z $UPLOAD_SYMBOLS ]; then
    echo "Sentry: Automatic symbols upload is disabled in plugin settings. Skipping..."
    exit
fi

if [ $UPLOAD_SYMBOLS != "True" ]; then
    echo "Sentry: Automatic symbols upload is disabled in plugin settings. Skipping..."
    exit
fi

INCLUDE_SOURCES=$(awk -F "=" '/IncludeSources/ {print $2}' "$DEFAULT_ENGINE_INI")

CLI_ARGS=()
if [ -z $INCLUDE_SOURCES -a $UPLOAD_SYMBOLS == "True" ]; then
    CLI_ARGS+=(--include-sources)
fi

CLI_LOG_LEVEL=$(awk -F "=" '/DiagnosticLevel/ {print $2}' "$DEFAULT_ENGINE_INI")

if [ -z $CLI_LOG_LEVEL ]; then
    CLI_LOG_LEVEL="info"
fi

ENABLED_TARGETS=$(grep "EnableBuildTargets" "$DEFAULT_ENGINE_INI" | sed -n 's/^EnableBuildTargets=//p' | sed -e 's/^(\(.*\))$/\1/')
if [ ! -z $ENABLED_TARGETS ]; then
    TARGETS_ARRAY=$(echo "$ENABLED_TARGETS" | sed -e 's/,/ /g')
    if [[ "${TARGETS_ARRAY[@]}" =~ "bEnable$targetType=False" ]]; then
        echo "Sentry: Automatic symbols upload is disabled for target type $targetType. Skipping..."
        exit
    fi
fi

ENABLED_CONFIGS=$(grep "EnableBuildConfigurations" "$DEFAULT_ENGINE_INI" | sed -n 's/^EnableBuildConfigurations=//p' | sed -e 's/^(\(.*\))$/\1/')
if [ ! -z $ENABLED_CONFIGS ]; then
    CONFIGS_ARRAY=$(echo "$ENABLED_CONFIGS" | sed -e 's/,/ /g')
    if [[ "${CONFIGS_ARRAY[@]}" =~ "bEnable$targetConfig=False" ]]; then
        echo "Sentry: Automatic symbols upload is disabled for build configuration $targetConfig. Skipping..."
        exit
    fi
fi

if [ $targetPlatform = "IOS" ] || [ $targetPlatform = "Mac" ]; then
    SENTRY_CLI_EXEC="$pluginPath/Source/ThirdParty/CLI/sentry-cli-Darwin-universal"
elif [ $targetPlatform = "Linux" ] || [ $targetPlatform = "LinuxArm64" ]; then
    SENTRY_CLI_EXEC="$pluginPath/Source/ThirdParty/CLI/sentry-cli-Linux-x86_64"
else
    echo "Sentry: Unexpected platform ${targetPlatform}. Skipping..."
    exit
fi

if [ ! -f "$SENTRY_CLI_EXEC" ]; then
    echo "Sentry: Sentry CLI is missing. Skipping..."
    exit
fi

PROPERTIES_FILE="$projectPath/sentry.properties"

echo "Sentry: Looking for properties file '$SENTRY_PROPERTIES'"

if [ -f "$PROPERTIES_FILE" ]; then
    echo "Sentry: Properties file found. Starting upload..."
    ProjectName=$(awk -F "=" '/defaults.project/ {print $2}' "$PROPERTIES_FILE")
    if [ -z "$ProjectName" ]; then        
        echo "Error: Project name is not set. Skipping..."
        exit
    fi
    OrgName=$(awk -F "=" '/defaults.org/ {print $2}' "$PROPERTIES_FILE")
    if [ -z "$OrgName" ]; then        
        echo "Error: Project organization is not set. Skipping..."
        exit
    fi
    AuthToken=$(awk -F "=" '/auth.token/ {print $2}' "$PROPERTIES_FILE")
    if [ -z "$AuthToken" ]; then        
        echo "Error: Auth token is not set. Skipping..."
        exit
    fi
    export SENTRY_PROPERTIES=$PROPERTIES_FILE
else
    echo "Sentry: Properties file not found. Falling back to environment variables."
    if [ -z "$SENTRY_PROJECT" ]; then        
        echo "Error: SENTRY_PROJECT env var is not set. Skipping..."
        exit
    fi
    if [ -z "$SENTRY_ORG" ]; then        
        echo "Error: SENTRY_ORG env var is not set. Skipping..."
        exit
    fi
    if [ -z "$SENTRY_AUTH_TOKEN" ]; then        
        echo "Error: SENTRY_AUTH_TOKEN env var is not set. Skipping..."
        exit
    fi
fi

chmod +x "$SENTRY_CLI_EXEC"

"$SENTRY_CLI_EXEC" debug-files upload $CLI_ARGS[@] --log-level $CLI_LOG_LEVEL "$PROJECT_BINARIES_PATH" "$PLUGIN_BINARIES_PATH"

echo "Sentry: Upload finished"
