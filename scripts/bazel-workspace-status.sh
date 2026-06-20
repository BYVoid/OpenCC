#!/usr/bin/env bash
set -euo pipefail

commit="$(git rev-parse HEAD 2>/dev/null || true)"
remote="$(git config --get remote.origin.url 2>/dev/null || true)"
dirty="0"
if ! git diff --quiet 2>/dev/null || ! git diff --cached --quiet 2>/dev/null; then
  dirty="1"
fi

if [[ -n "$commit" ]]; then
  printf 'STABLE_BUILD_SCM_REVISION %s\n' "$commit"
fi
if [[ -n "$remote" ]]; then
  printf 'STABLE_BUILD_SCM_REMOTE %s\n' "$remote"
fi
printf 'STABLE_BUILD_SCM_DIRTY %s\n' "$dirty"
printf 'BUILD_TIMESTAMP %s\n' "$(date -u +%s)"
