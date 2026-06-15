#!/usr/bin/env bash
set -euo pipefail

print_space_freed() {
    local step_name="$1"
    local current_space
    current_space=$(df -BM --output=avail / | tail -1 | tr -dc '0-9')
    local freed=$((current_space - last_reported_space))
    local freed_gb
    local current_gb
    freed_gb=$(awk "BEGIN { printf \"%.2f\", $freed / 1024 }")
    current_gb=$(awk "BEGIN { printf \"%.2f\", $current_space / 1024 }")
    echo "After $step_name - Freed: ${freed_gb} GB, Current free space: ${current_gb} GB"
    last_reported_space=$current_space
    total_freed=$((total_freed + freed))
}

initial_space=$(df -BM --output=avail / | tail -1 | tr -dc '0-9')
last_reported_space=$initial_space
total_freed=0

initial_gb=$(awk "BEGIN { printf \"%.2f\", $initial_space / 1024 }")
echo "Initial free space: ${initial_gb} GB"

sudo swapoff -a || true
sudo rm -f /swapfile
print_space_freed "Swap removal"

sudo rm -rf /usr/local/lib/android
print_space_freed "Android SDK removal"

sudo rm -rf /usr/share/dotnet
print_space_freed ".NET removal"

sudo rm -rf /usr/share/swift
print_space_freed "Swift removal"

sudo rm -rf /usr/local/share/powershell
print_space_freed "PowerShell removal"

sudo rm -rf /usr/local/.ghcup
sudo rm -rf /opt/ghc
print_space_freed "Haskell removal"

sudo rm -rf /usr/local/lib/node_modules
print_space_freed "Node modules removal"

sudo rm -rf /usr/local/share/boost
print_space_freed "Boost removal"

sudo rm -rf /usr/lib/google-cloud-sdk
print_space_freed "Google Cloud SDK removal"

sudo rm -rf /usr/lib/jvm
print_space_freed "JVM removal"

sudo rm -rf /opt/pipx
print_space_freed "pipx removal"

if [[ -n "${AGENT_TOOLSDIRECTORY:-}" ]]; then
    sudo rm -rf "$AGENT_TOOLSDIRECTORY"
    print_space_freed "Agent toolcache removal"
fi

sudo apt-get clean
sudo rm -rf /var/lib/apt/lists/*
print_space_freed "APT cleanup"

final_space=$(df -BM --output=avail / | tail -1 | tr -dc '0-9')
final_gb=$(awk "BEGIN { printf \"%.2f\", $final_space / 1024 }")
total_freed_gb=$(awk "BEGIN { printf \"%.2f\", $total_freed / 1024 }")

echo "Initial free space: ${initial_gb} GB"
echo "Final free space: ${final_gb} GB"
echo "Total space gained: ${total_freed_gb} GB"
