#!/usr/bin/env bash
#
# compute-version.sh — Derive a version string from Git state.
#
# Output format:
#   Tag build (release):    v1.2.3
#   Dev build (clean):      v1.2.3.dev4+gabc1234
#   Dev build (dirty):      v1.2.3.dev4+gabc1234.dirty
#   No tag (clean):         gabc1234
#   No tag (dirty):         gabc1234.dirty
#
# Usage:
#   ./scripts/compute-version.sh          # prints version to stdout
#   VERSION=$(./scripts/compute-version.sh)
#
set -euo pipefail

# Detect dirty working tree
DIRTY=""
if ! git diff --quiet 2>/dev/null || ! git diff --cached --quiet 2>/dev/null; then
  DIRTY=".dirty"
fi

# Get raw git describe output
RAW=$(git describe --tags --long --always 2>/dev/null) || RAW=""

if [ -z "$RAW" ]; then
  # No git available at all — fall back to unknown
  SHA=$(git rev-parse --short HEAD 2>/dev/null) || SHA="unknown"
  echo "g${SHA}${DIRTY}"
  exit 0
fi

# Case 1: Exact tag (e.g. v1.2.3-0-gabc1234 from --long)
if [[ "$RAW" =~ ^(v[0-9]+\.[0-9]+\.[0-9]+)-0-g[0-9a-f]+$ ]]; then
  TAG="${BASH_REMATCH[1]}"
  echo "${TAG}${DIRTY}"
  exit 0
fi

# Case 2: Dev build with tag (e.g. v1.2.3-4-gabc1234)
if [[ "$RAW" =~ ^(v[0-9]+\.[0-9]+\.[0-9]+)-([0-9]+)-g([0-9a-f]+)$ ]]; then
  BASE="${BASH_REMATCH[1]}"
  COUNT="${BASH_REMATCH[2]}"
  SHA="${BASH_REMATCH[3]}"
  echo "${BASE}.dev${COUNT}+g${SHA}${DIRTY}"
  exit 0
fi

# Case 3: No tag — raw is just a short SHA
echo "g${RAW}${DIRTY}"
