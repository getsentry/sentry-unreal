#!/bin/bash

export targetPlatform=$1
export targetName=$2
export targetType=$3
export projectPath=$4
export pluginPath=$5

BINARIES_PATH=$projectPath/Binaries/$targetPlatform
CONFIG_PATH=$projectPath/Config

echo "Sentry: Start debug symbols upload"

if [ $targetType = "Editor" ]; then
    echo "Sentry: Automatic symbols upload is not required for Editor target. Terminating..."
    exit
fi

if [ $targetPlatform = "IOS" ] || [ $targetPlatform = "Mac" ]; then
    SENTRY_CLI_EXEC=$pluginPath/Source/ThirdParty/CLI/sentry-cli-Darwin-universal
elif [ $targetPlatform = "Linux" ]; then
    SENTRY_CLI_EXEC=$pluginPath/Source/ThirdParty/CLI/sentry-cli-Linux-x86_64
else
    echo "Sentry: Unexpected platform ${targetPlatform}. Terminating..."
    exit
fi

UPLOAD_SYMBOLS=$(awk -F "=" '/UploadSymbolsAutomatically/ {print $2}' ${CONFIG_PATH}/DefaultEngine.ini)

if [ -z $UPLOAD_SYMBOLS ]; then
    echo "Sentry: Automatic symbols upload is disabled in plugin settings. Terminating..."
    exit
fi

if [ $UPLOAD_SYMBOLS != "True" ]; then
    echo "Sentry: Automatic symbols upload is disabled in plugin settings. Terminating..."
    exit
fi

echo "Sentry: parse project settings"

PROJECT_NAME=$(awk -F "=" '/ProjectName/ {print $2}' ${CONFIG_PATH}/DefaultEngine.ini)
ORG_NAME=$(awk -F "=" '/OrganisationName/ {print $2}' ${CONFIG_PATH}/DefaultEngine.ini)
AUTH_TOKEN=$(awk -F "=" '/AuthToken/ {print $2}' ${CONFIG_PATH}/DefaultEngine.ini)

echo "Sentry: Copy user credentials config file template to home directory"
cp $pluginPath/Resources/sentry.properties $BINARIES_PATH/sentry.properties

sed -i.backup 's/your-project/'$PROJECT_NAME'/g' $BINARIES_PATH/sentry.properties
sed -i.backup 's/your-org/'$ORG_NAME'/g' $BINARIES_PATH/sentry.properties
sed -i.backup 's/YOUR_AUTH_TOKEN/'$AUTH_TOKEN'/g' $BINARIES_PATH/sentry.properties

export SENTRY_PROPERTIES=$BINARIES_PATH/sentry.properties

chmod +x $SENTRY_CLI_EXEC

$SENTRY_CLI_EXEC upload-dif --include-sources $BINARIES_PATH
