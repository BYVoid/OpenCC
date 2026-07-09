#!/usr/bin/env bash
#
# Build opencc.node with Bazel and copy it to the npm prebuilds directory.
# Usage: ./scripts/build-node-prebuild-bazel.sh <target>

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

# Git Bash/MSYS rewrites Bazel labels such as //node:opencc as paths on Windows.
case "$(uname -s 2>/dev/null || true)" in
  MINGW*|MSYS*|CYGWIN*)
    export MSYS2_ARG_CONV_EXCL='*'
    export MSYS_NO_PATHCONV=1
    ;;
esac

usage() {
  cat >&2 <<'EOF'
Usage: ./scripts/build-node-prebuild-bazel.sh <target>

Supported targets:
  darwin-arm64
  darwin-x64
  linux-arm64
  linux-x64
  win32-x64
EOF
}

if [[ $# -ne 1 ]]; then
  usage
  exit 2
fi

target="$1"
case "$target" in
  darwin-arm64|darwin-x64|linux-arm64|linux-x64|win32-x64)
    ;;
  *)
    echo "Unsupported target: $target" >&2
    usage
    exit 2
    ;;
esac

host_target="$(node -p "process.platform + '-' + process.arch")"

if [[ "$target" == "win32-x64" && "$host_target" != "$target" ]]; then
  echo "Building $target with Bazel + Zig cross-compile..."
  bazel build -c opt //data/dictionary:binary_dictionaries
  bazel build -c opt --config=node-windows-x64-zig //node:opencc_node_windows_zig
  addon="bazel-bin/node/windows-x64/opencc.node"
elif [[ "$target" == "$host_target" ]]; then
  echo "Building $target with native Bazel toolchain..."
  bazel build -c opt //data/dictionary:binary_dictionaries //node:opencc
  addon="bazel-bin/node/opencc.node"
else
  cat >&2 <<EOF
Target $target does not match host $host_target.
Run this script on a matching runner, or use win32-x64 from macOS/Linux via the
existing Bazel + Zig cross-compile path.
EOF
  exit 1
fi

echo "Preparing shared Node.js prebuild assets..."
node scripts/prepare-node-prebuild-artifacts.js

dest_dir="prebuilds/$target"
dest="$dest_dir/opencc.node"
mkdir -p "$dest_dir"
cp -fL "$addon" "$dest"

echo "Copied Bazel-built addon to $dest"
ls -lh "$dest"
