#!/usr/bin/env bash
#
# compute-version.sh — Derive a version string from Git state.
#
# Output format:
#   Tag build (release):    v1.2.3
#   Dev build (clean):      v1.2.3.dev4+gabc1234
#   Dev build (dirty):      v1.2.3.dev4+gabc1234.dirty
#   No tag (clean):         v1.2.1+gabc1234
#   No tag (dirty):         v1.2.1+gabc1234.dirty
#
# Usage:
#   ./scripts/compute-version.sh          # prints version to stdout
#   VERSION=$(./scripts/compute-version.sh)
#
set -euo pipefail

# Fallback version — must be kept in sync with cmake/GitVersion.cmake
FALLBACK_VERSION="1.2.1"

# Detect dirty working tree
DIRTY=""
if ! git diff --quiet 2>/dev/null || ! git diff --cached --quiet 2>/dev/null; then
  DIRTY=".dirty"
fi

# Get raw git describe output
RAW=$(git describe --tags --long --always 2>/dev/null) || RAW=""

if [ -z "$RAW" ]; then
  # No git available at all — use fallback version
  SHA=$(git rev-parse --short HEAD 2>/dev/null) || SHA=""
  if [ -n "$SHA" ]; then
    echo "v${FALLBACK_VERSION}+g${SHA}${DIRTY}"
  else
    echo "v${FALLBACK_VERSION}${DIRTY}"
  fi
  exit 0
fi

# Case 1: Exact tag (e.g. v1.2.3-0-gabc1234 or ver.1.2.3-0-gabc1234 from --long)
# NOTE: version tag patterns here must be kept in sync with cmake/GitVersion.cmake
if [[ "$RAW" =~ ^(v|ver\.)([0-9]+\.[0-9]+\.[0-9]+)-0-g[0-9a-f]+$ ]]; then
  BASE="${BASH_REMATCH[2]}"
  echo "v${BASE}${DIRTY}"
  exit 0
fi

# Case 2: Dev build with tag (e.g. v1.2.3-4-gabc1234 or ver.1.2.3-4-gabc1234)
if [[ "$RAW" =~ ^(v|ver\.)([0-9]+\.[0-9]+\.[0-9]+)-([0-9]+)-g([0-9a-f]+)$ ]]; then
  BASE="${BASH_REMATCH[2]}"
  COUNT="${BASH_REMATCH[3]}"
  SHA="${BASH_REMATCH[4]}"
  echo "v${BASE}.dev${COUNT}+g${SHA}${DIRTY}"
  exit 0
fi

# Case 3: No tag — use fallback version with commit SHA
echo "v${FALLBACK_VERSION}+g${RAW}${DIRTY}"
