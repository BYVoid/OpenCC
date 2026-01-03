#!/usr/bin/env bash
set -euo pipefail

# Regenerate wasm-lib assets from Bazel outputs:
#  - data/dictionary/*.ocd2  -> wasm-lib/data/dict/
#  - test/testcases.json      -> wasm-lib/test/testcases.json

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}/.."

echo "Building dictionaries via Bazel..."
bazel build //data/dictionary:binary_dictionaries

BAZEL_BIN="$(bazel info bazel-bin)"

DICT_SRC="${BAZEL_BIN}/data/dictionary"
DICT_DST="${ROOT}/data/dict"
mkdir -p "${DICT_DST}"
# Ensure target writable (some checked-in artifacts may be read-only)
chmod -R u+w "${DICT_DST}"
echo "Collecting required .ocd2 names from data/config/*.json"
NEEDED_DICTS=()
while IFS= read -r line; do
  [[ -n "$line" ]] && NEEDED_DICTS+=("$line")
done <<< "$(rg -o '"file"\\s*:\\s*"[^"]+\\.ocd2"' data/config/*.json \
  | sed -E 's/.*"file"\\s*:\\s*"([^"]+)".*/\\1/' | sort -u)"

# If no matches (unexpected), fall back to all .ocd2
if [[ ${#NEEDED_DICTS[@]} -eq 0 ]]; then
  echo "No referenced dicts found via config scan; copying all .ocd2"
  while IFS= read -r line; do
    [[ -n "$line" ]] && NEEDED_DICTS+=("$line")
  done <<< "$(cd "${DICT_SRC}" && ls *.ocd2)"
fi

echo "Refreshing dicts in ${DICT_DST}"
rm -f "${DICT_DST}"/*.ocd2
for f in "${NEEDED_DICTS[@]}"; do
  install -m 644 "${DICT_SRC}/${f}" "${DICT_DST}/${f}"
done

CASE_SRC="${ROOT}/../test/testcases/testcases.json"
CASE_DST="${ROOT}/test/testcases.json"
mkdir -p "$(dirname "${CASE_DST}")"
# Remove any old JSON to avoid stale copies
rm -f "${CASE_DST}"
echo "Copying testcases.json from ${CASE_SRC} -> ${CASE_DST}"
install -m 644 "${CASE_SRC}" "${CASE_DST}"

echo "Done."
