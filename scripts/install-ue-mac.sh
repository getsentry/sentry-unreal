#!/bin/sh
# Installs the pre-built Unreal Engine for macOS that the ue-oci-macos workflow
# publishes to GHCR (a plain OCI artifact holding the engine zip split into
# parts, so it can only be fetched with oras) and extracts it to $UE_ROOT.
set -eu

: "${UNREAL_VERSION:?}" "${UE_ROOT:?}" "${GITHUB_TOKEN:?}" "${GITHUB_ACTOR:?}" "${RUNNER_TEMP:?}"

registry=ghcr.io
artifact="$registry/getsentry/unreal-docker:$UNREAL_VERSION-mac"

work_dir=$(mktemp -d "$RUNNER_TEMP/ue.XXXXXX")
cd "$work_dir"

printf '%s\n' "$GITHUB_TOKEN" | oras login "$registry" -u "$GITHUB_ACTOR" --password-stdin

# Retry the pull - the runner's network occasionally drops connections mid-download
attempt=1
until oras pull "$artifact"; do
  if [ "$attempt" -ge 3 ]; then
    printf '::error::Failed to pull %s\n' "$artifact" >&2
    exit 1
  fi
  attempt=$((attempt + 1))
  rm -f ue.zip.part-*
  sleep 10
done

# Reassemble one part at a time so the parts and the full archive never sit on disk together
for part in ue.zip.part-*; do
  cat "$part" >> ue.zip
  rm "$part"
done

mkdir -p "$UE_ROOT"
unzip -q ue.zip -d "$UE_ROOT"
rm ue.zip

# Zip archives may not preserve execute permissions
chmod -R u+x "$UE_ROOT/Engine/Build/BatchFiles" "$UE_ROOT/Engine/Binaries"

df -h "$UE_ROOT"
