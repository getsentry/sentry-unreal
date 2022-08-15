#!/bin/bash
set -euo pipefail

git submodule foreach git submodule update --init --recursive