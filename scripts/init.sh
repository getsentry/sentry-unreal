#!/usr/bin/env bash
set -euo pipefail

case "$(uname -s)" in
CYGWIN* | MINGW32* | MSYS* | MINGW*)
    echo "This script is not supposed to be run on Windows. Use init-win.ps1 instead."
    exit 1
    ;;
esac

repoRoot=$(
    cd "$(dirname "$0")/.."
    pwd -P
)

setupPluginLink() {
    targetDir="$repoRoot/plugin-dev"
    linkPath="$repoRoot/$1/Plugins/sentry"

    if [[ -L "$linkPath" ]]; then
        if [[ "$(readlink -f $linkPath)" == "$targetDir" ]]; then
            echo "Link $linkPath already exists and matches target $targetDir - skipping"
            return
        fi
        echo "Removing existing link $linkPath because it doesn't match the expected target"
        unlink "$linkPath"
    fi

    mkdir -p "$(dirname "$linkPath")"
    echo "Creating a link from $linkPath to $targetDir"
    ln -s "$targetDir" "$linkPath"
}

setupPluginLink "sample"
"$(dirname "$0")/download-sdks.sh"
"$(dirname "$0")/download-cli.sh"
