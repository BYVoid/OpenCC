#!/usr/bin/env bash
#
# Build Windows x64 opencc.node using Zig and copy it to the prebuilds directory.
# Usage: ./scripts/build-node-windows-prebuild-zig.sh

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

exec ./scripts/build-node-prebuild-bazel.sh win32-x64
