#!/usr/bin/env bash
set -euo pipefail

function getProperty {
    cat "$(dirname $0)/../plugin-dev/sentry-cli.properties" | grep $1 | cut -d'=' -f2 | tr -d '"'
}

baseUrl="$(getProperty 'repo')/releases/download/cli@$(getProperty 'version')/sentry-"
targetDir="$(dirname $0)/../plugin-dev/Source/ThirdParty/CLI"

if ! [ -d "$targetDir" ]; then
    mkdir $targetDir
fi

cd $targetDir

# Remove previous contents of the directory (remove all ignored files).
git clean -fXd .

declare -a platforms=("darwin-arm64" "linux-x64" "windows-x64.exe")
for platform in "${platforms[@]}"; do
    echo "Downloading Sentry CLI for $platform to $PWD ..."
    curl -fLJO $baseUrl$platform
    chmod +x "sentry-$platform"
done
