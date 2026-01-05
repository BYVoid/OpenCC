#!/usr/bin/env bash
set -euo pipefail

# Regenerate wasm-lib assets from Bazel outputs:
#  - data/dictionary/*.ocd2       -> wasm-lib/data/dict/
#  - data/config/*.json           -> wasm-lib/data/config/
#  - test/testcases/testcases.json -> wasm-lib/test/testcases.json
#  - test/testcases/cngov_testcases.json -> wasm-lib/test/cngov_testcases.json

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
done <<< "$(
  grep -h -o '"file"[[:space:]]*:[[:space:]]*"[^"]*\\.ocd2"' data/config/*.json \
    | sed -E 's/.*"file"[[:space:]]*:[[:space:]]*"([^"]+)".*/\\1/' | sort -u
)"

# If no matches (unexpected), fall back to all .ocd2
if [[ ${#NEEDED_DICTS[@]} -eq 0 ]]; then
  echo "No referenced dicts found via config scan; copying all .ocd2"
  while IFS= read -r line; do
    [[ -n "$line" ]] && NEEDED_DICTS+=("$line")
  done <<< "$(cd "${DICT_SRC}" && find . \
    -name '*runfiles*' -prune -o \
    -name '*.ocd2' -print \
    | sed -E 's|^\\./||' | sort -u)"
fi

echo "Refreshing dicts in ${DICT_DST}"
# Clear out any stale files/dirs so only expected .ocd2 remain.
find "${DICT_DST}" -mindepth 1 -exec rm -rf {} +
for f in "${NEEDED_DICTS[@]}"; do
  if [[ "${f}" == *runfiles* ]]; then
    echo "Skipping runfiles entry ${f}" >&2
    continue
  fi
  src="${DICT_SRC}/${f}"
  dst="${DICT_DST}/${f}"
  if [[ ! -f "${src}" ]]; then
    echo "Warning: missing dict source ${src}" >&2
    continue
  fi
  # Handle subdirectory paths like cngov/TGCharacters.ocd2
  mkdir -p "$(dirname "${dst}")"
  install -m 644 "${src}" "${dst}"
done

CONFIG_SRC="${ROOT}/../data/config"
CONFIG_DST="${ROOT}/data/config"
mkdir -p "${CONFIG_DST}"
chmod -R u+w "${CONFIG_DST}"
rm -f "${CONFIG_DST}"/*.json
echo "Copying config JSON from ${CONFIG_SRC} -> ${CONFIG_DST}"
install -m 644 "${CONFIG_SRC}"/*.json "${CONFIG_DST}/"

CASE_SRC="${ROOT}/../test/testcases/testcases.json"
CASE_DST="${ROOT}/test/testcases.json"
mkdir -p "$(dirname "${CASE_DST}")"
# Remove any old JSON to avoid stale copies
rm -f "${CASE_DST}"
echo "Copying testcases.json from ${CASE_SRC} -> ${CASE_DST}"
install -m 644 "${CASE_SRC}" "${CASE_DST}"

CNGOV_CASE_SRC="${ROOT}/../test/testcases/cngov_testcases.json"
CNGOV_CASE_DST="${ROOT}/test/cngov_testcases.json"
echo "Copying cngov_testcases.json from ${CNGOV_CASE_SRC} -> ${CNGOV_CASE_DST}"
install -m 644 "${CNGOV_CASE_SRC}" "${CNGOV_CASE_DST}"

echo "Done."
