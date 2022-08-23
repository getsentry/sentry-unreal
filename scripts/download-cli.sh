#!/usr/bin/env bash
set -euo pipefail

function getProperty {
    cat "$(dirname $0)/../modules/sentry-cli.properties" | grep $1 | cut -d'=' -f2 | tr -d '"'
}

baseUrl="$(getProperty 'repo')/releases/download/$(getProperty 'version')/sentry-cli-"

mkdir -p "$(dirname $0)/../plugin-dev/Source/ThirdParty"
cd "$(dirname $0)/../plugin-dev/Source/ThirdParty"
mkdir -p "CLI"
cd "CLI"

declare -a platforms=("Darwin-universal" "Linux-x86_64" "Windows-x86_64.exe")
for platform in "${platforms[@]}"; do
    echo "Downloading Sentry CLI for $platform to $PWD ..."
    curl -LJO $baseUrl$platform
    chmod +x "sentry-cli-$platform"
done