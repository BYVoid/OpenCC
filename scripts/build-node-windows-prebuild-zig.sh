#!/usr/bin/env bash
#
# Build Windows x64 opencc.node using Zig and copy it to the prebuilds directory.
# Usage: ./scripts/build-node-windows-prebuild-zig.sh

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "Building Windows x64 (node-windows-x64-zig)..."
bazel build --config=node-windows-x64-zig //node:opencc_node_windows_zig

mkdir -p prebuilds/win32-x64
cp -fL bazel-bin/node/windows-x64/opencc.node prebuilds/win32-x64/
echo "Copied to prebuilds/win32-x64/opencc.node"

ls -lh prebuilds/win32-x64/opencc.node
