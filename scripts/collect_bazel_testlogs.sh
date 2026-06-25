#!/usr/bin/env bash

set -euo pipefail

artifact_dir="${1:-bazel-testlogs-artifact}"

testlogs_dir="$(bazel info bazel-testlogs 2>/dev/null || true)"
if [ -z "${testlogs_dir}" ] || [ ! -d "${testlogs_dir}" ]; then
  echo "No Bazel testlogs directory found."
  exit 0
fi

rm -rf "${artifact_dir}"
mkdir -p "${artifact_dir}"

while IFS= read -r -d '' file; do
  relative_path="${file#"${testlogs_dir}/"}"
  destination="${artifact_dir}/${relative_path}"
  mkdir -p "$(dirname "${destination}")"
  cp "${file}" "${destination}"
done < <(
  find "${testlogs_dir}" \
    \( -name test.log -o -name test.xml -o -name outputs.zip -o -name test.outputs_manifest \) \
    -type f -print0
)

echo "Collected Bazel test logs into ${artifact_dir}"
