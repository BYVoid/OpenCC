#!/usr/bin/env bash
#
# Build opencc.node via Bazel remote execution and copy to the prebuilds directory.
# Usage: ./scripts/build-node-prebuild-bazel.sh

set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "Building Linux x64 (node-linux-x64)..."
bazel build --config=node-linux-x64 -c opt --remote_download_toplevel //node:opencc_node
mkdir -p prebuilds/linux-x64
cp -fL bazel-bin/node/opencc.node prebuilds/linux-x64/
echo "Copied to prebuilds/linux-x64/opencc.node"

echo ""
echo "Building Linux arm64 (node-linux-arm64)..."
bazel build --config=node-linux-arm64 -c opt --remote_download_toplevel //node:opencc_node
mkdir -p prebuilds/linux-arm64
cp -fL bazel-bin/node/opencc.node prebuilds/linux-arm64/
echo "Copied to prebuilds/linux-arm64/opencc.node"

# 资产文件仍然依赖于 prepare-node-prebuild-artifacts.js，
# 但那是 npm run prebuild 的一部分。如果需要，也可以在这里复制。
echo ""
echo "Prebuild files collected successfully."
ls -lh prebuilds/linux-x64/opencc.node prebuilds/linux-arm64/opencc.node
