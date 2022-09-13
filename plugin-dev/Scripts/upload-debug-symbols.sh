#!/bin/bash

targetPlatform=$1
targetName=$2
targetType=$3
projectPath=$4
pluginPath=$5

PROJECT_BINARIES_PATH=$projectPath/Binaries/$targetPlatform
PLUGIN_BINARIES_PATH=$pluginPath/Source/ThirdParty/$targetPlatform
CONFIG_PATH=$projectPath/Config

echo "Sentry: Start debug symbols upload"

if [ $targetType = "Editor" ]; then
    echo "Sentry: Automatic symbols upload is not required for Editor target. Skipping..."
    exit
fi

if [ $targetPlatform = "IOS" ] || [ $targetPlatform = "Mac" ]; then
    SENTRY_CLI_EXEC=$pluginPath/Source/ThirdParty/CLI/sentry-cli-Darwin-universal
elif [ $targetPlatform = "Linux" ]; then
    SENTRY_CLI_EXEC=$pluginPath/Source/ThirdParty/CLI/sentry-cli-Linux-x86_64
else
    echo "Sentry: Unexpected platform ${targetPlatform}. Skipping..."
    exit
fi

UPLOAD_SYMBOLS=$(awk -F "=" '/UploadSymbolsAutomatically/ {print $2}' ${CONFIG_PATH}/DefaultEngine.ini)

if [ -z $UPLOAD_SYMBOLS ]; then
    echo "Sentry: Automatic symbols upload is disabled in plugin settings. Skipping..."
    exit
fi

if [ $UPLOAD_SYMBOLS != "True" ]; then
    echo "Sentry: Automatic symbols upload is disabled in plugin settings. Skipping..."
    exit
fi

export SENTRY_PROPERTIES="$projectPath/sentry.properties"
if [ ! -f "$SENTRY_PROPERTIES" ]; then
    echo "Sentry: Properties file is missing: '$SENTRY_PROPERTIES'"
    exit 1
fi

export SENTRY_PROPERTIES="$projectPath/sentry.properties"

echo "Sentry: Upload started using PropertiesFile '$SENTRY_PROPERTIES'"

chmod +x $SENTRY_CLI_EXEC

$SENTRY_CLI_EXEC upload-dif --include-sources $PROJECT_BINARIES_PATH $PLUGIN_BINARIES_PATH

echo "Sentry: Upload finished"
