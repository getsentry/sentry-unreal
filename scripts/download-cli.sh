#!/usr/bin/env bash
set -euo pipefail

mkdir -p "$(dirname $0)/../plugin-dev/Source/ThirdParty"
cd "$(dirname $0)/../plugin-dev/Source/ThirdParty"
mkdir -p "CLI"
cd "CLI"

function getProperty {
    cat "../modules/sentry-cli.properties" | grep $1 | cut -d'=' -f2 | tr -d '"'
}

baseUrl="$(getProperty 'repo')/releases/download/$(getProperty 'version')/sentry-cli-"

declare -a platforms=("Darwin-universal" "Linux-x86_64" "Windows-x86_64")
for platform in "${platforms[@]}"; do
    echo "Downloading Sentry CLI for $platform to $PWD ..."
    curl -LI $baseUrl$platform
done