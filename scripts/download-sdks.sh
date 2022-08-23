#!/usr/bin/env bash
set -euo pipefail

echo "Downloading native SDKs from the latest CI pipeline"

mkdir -p "$(dirname $0)/../plugin-dev/Source/ThirdParty"
cd "$(dirname $0)/../plugin-dev/Source/ThirdParty"

findCiRun() {
    echo "Looking for the latest successful CI run on branch '$1'" >/dev/stderr
    id=$(gh run list --branch $1 --workflow package-plugin-workflow \
        --json 'conclusion,databaseId' --jq 'first(.[] | select(.conclusion == "success") | .databaseId)')
    if [[ "$id" == "" ]]; then
        echo "  ... no successful CI run found on $1" >/dev/stderr
        return 1
    else
        echo "  ... found CI run ID: $id" >/dev/stderr
        echo "$id"
        return 0
    fi
}

runId=$(findCiRun "$(git rev-parse --abbrev-ref HEAD)" || findCiRun main)
if [[ "$runId" == "" ]]; then
    exit 1
fi

declare -a sdks=("Android" "IOS" "Linux" "Mac" "Win64")
for sdk in "${sdks[@]}"; do
    echo "Downloading $sdk SDK to $PWD/$sdk ..."
    rm -rf "./$sdk"
    gh run download $runId -n "$sdk-sdk" -D $sdk
    chmod -R +x $sdk
done
