#!/bin/bash

targetPlatform=$1
targetName=$2
targetType=$3
targetConfig=$4
projectPath=$5
pluginPath=$6

PROJECT_BINARIES_PATH="$projectPath/Binaries/$targetPlatform"
PLUGIN_BINARIES_PATH="$pluginPath/Source/ThirdParty/$targetPlatform"
CONFIG_PATH="$projectPath/Config"

echo "Sentry: Start debug symbols upload"

if [ $targetType = "Editor" ]; then
    echo "Sentry: Automatic symbols upload is not required for Editor target. Skipping..."
    exit
fi

if [ $targetPlatform = "IOS" ] || [ $targetPlatform = "Mac" ]; then
    SENTRY_CLI_EXEC="$pluginPath/Source/ThirdParty/CLI/sentry-cli-Darwin-universal"
elif [ $targetPlatform = "Linux" ] || [ $targetPlatform = "LinuxArm64" ]; then
    SENTRY_CLI_EXEC="$pluginPath/Source/ThirdParty/CLI/sentry-cli-Linux-x86_64"
elif [ $targetPlatform = "Android" ]; then
    echo "Sentry: Debug symbols upload for Android is handled by Sentry's gradle plugin if enabled"
    exit
else
    echo "Sentry: Unexpected platform ${targetPlatform}. Skipping..."
    exit
fi

UPLOAD_SYMBOLS=$(awk -F "=" '/UploadSymbolsAutomatically/ {print $2}' "${CONFIG_PATH}/DefaultEngine.ini")

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

INCLUDE_SOURCES=$(awk -F "=" '/IncludeSources/ {print $2}' "${CONFIG_PATH}/DefaultEngine.ini")

CLI_ARGS=()
if [ -z $INCLUDE_SOURCES -a $UPLOAD_SYMBOLS == "True" ]; then
    CLI_ARGS+=(--include-sources)
fi

CLI_LOG_LEVEL=$(awk -F "=" '/DiagnosticLevel/ {print $2}' "${CONFIG_PATH}/DefaultEngine.ini")

if [ -z $CLI_LOG_LEVEL ]; then
    CLI_LOG_LEVEL="info"
fi

ENABLED_PLATFORMS=$(grep "EnableBuildPlatforms" "${CONFIG_PATH}/DefaultEngine.ini" | sed -n 's/^EnableBuildPlatforms=//p' | sed -e 's/^(\(.*\))$/\1/')
if [ ! -z $ENABLED_PLATFORMS ]; then
    PLATFORMS_ARRAY=$(echo "$ENABLED_PLATFORMS" | sed -e 's/,/ /g')
    if [[ "${PLATFORMS_ARRAY[@]}" =~ "bEnable$targetPlatform=False" ]]; then
        echo "Sentry: Automatic symbols upload is disabled for build platform $targetPlatform. Skipping..."
        exit
    fi
fi

ENABLED_TARGETS=$(grep "EnableBuildTargets" "${CONFIG_PATH}/DefaultEngine.ini" | sed -n 's/^EnableBuildTargets=//p' | sed -e 's/^(\(.*\))$/\1/')
if [ ! -z $ENABLED_TARGETS ]; then
    TARGETS_ARRAY=$(echo "$ENABLED_TARGETS" | sed -e 's/,/ /g')
    if [[ "${TARGETS_ARRAY[@]}" =~ "bEnable$targetType=False" ]]; then
        echo "Sentry: Automatic symbols upload is disabled for target type $targetType. Skipping..."
        exit
    fi
fi

ENABLED_CONFIGS=$(grep "EnableBuildConfigurations" "${CONFIG_PATH}/DefaultEngine.ini" | sed -n 's/^EnableBuildConfigurations=//p' | sed -e 's/^(\(.*\))$/\1/')
if [ ! -z $ENABLED_CONFIGS ]; then
    CONFIGS_ARRAY=$(echo "$ENABLED_CONFIGS" | sed -e 's/,/ /g')
    if [[ "${CONFIGS_ARRAY[@]}" =~ "bEnable$targetConfig=False" ]]; then
        echo "Sentry: Automatic symbols upload is disabled for build configuration $targetConfig. Skipping..."
        exit
    fi
fi

export SENTRY_PROPERTIES="$projectPath/sentry.properties"
if [ ! -f "$SENTRY_PROPERTIES" ]; then
    echo "Sentry: Properties file is missing: '$SENTRY_PROPERTIES'"
    exit
fi

if [ ! -f "$SENTRY_CLI_EXEC" ]; then
    echo "Sentry: Sentry CLI is not configured in plugin settings. Skipping..."
    exit
fi

echo "Sentry: Upload started using PropertiesFile '$SENTRY_PROPERTIES'"

chmod +x "$SENTRY_CLI_EXEC"

"$SENTRY_CLI_EXEC" upload-dif $CLI_ARGS[@] --log-level $CLI_LOG_LEVEL "$PROJECT_BINARIES_PATH" "$PLUGIN_BINARIES_PATH"

echo "Sentry: Upload finished"
