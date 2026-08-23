#!/usr/bin/env bash
#
# Build and smoke-test a trimmed CPack source package.
#
# Usage:
#   bash scripts/build-source-package.sh build <profile>
#   bash scripts/build-source-package.sh smoke-test <profile> [tarball]
#   bash scripts/build-source-package.sh all <profile>
#
# Profiles (see OPENCC_SOURCE_PACKAGE_PROFILE in CMakeLists.txt):
#   opencc  C++ core and Jieba plugin only.
#   bazel   C++ core, Jieba plugin, and the Node.js/Python binding sources.
#
# Both profiles are buildable with CMake and Bazel, so the smoke test exercises
# both build systems; the "bazel" profile additionally builds and tests the
# //node and //python targets, mirroring the target patterns used by
# .github/workflows/bazel.yml.
#
# Environment variables:
#   DIST_DIR   Directory to write the final archives (default: <repo>/dist)
#   BUILD_DIR  CMake binary directory used to run CPack (default: <repo>/build-src)
#   WORK_DIR   Directory the package is extracted into for the smoke test
#              (default: a fresh mktemp -d)
#   TAR        GNU tar binary used for the reproducible repack (default: tar,
#              falling back to gtar; BSD tar lacks --sort/--owner/--mtime)

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="${DIST_DIR:-$ROOT_DIR/dist}"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build-src}"

usage() {
  sed -n '3,25p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
}

# Package name suffix produced by each profile (must match CMakeLists.txt).
profile_suffix() {
  case "$1" in
  opencc) echo "opencc-src" ;;
  bazel) echo "bazel-src" ;;
  *)
    echo "Unknown profile: $1 (expected 'opencc' or 'bazel')" >&2
    exit 1
    ;;
  esac
}

find_tarball() {
  local suffix
  suffix="$(profile_suffix "$1")"
  local tarball
  tarball="$(ls "$DIST_DIR"/opencc-*-"$suffix".tar.gz 2>/dev/null | head -n 1 || true)"
  if [ -z "$tarball" ]; then
    echo "No source package found in $DIST_DIR for profile '$1'." >&2
    exit 1
  fi
  echo "$tarball"
}

# Locate a GNU tar; the reproducible repack needs --sort/--owner/--group/--mtime.
gnu_tar() {
  local candidate
  for candidate in "${TAR:-}" tar gtar; do
    [ -n "$candidate" ] || continue
    if command -v "$candidate" >/dev/null 2>&1 && "$candidate" --version 2>/dev/null | grep -q GNU; then
      echo "$candidate"
      return 0
    fi
  done
  echo "GNU tar is required for the reproducible repack (install gnu-tar and/or set TAR)." >&2
  return 1
}

# Build the CPack source archive, then repack it for reproducibility:
# normalized ownership (root:root), timestamps (epoch 0), and file order.
build_package() {
  local profile="$1"
  local suffix
  suffix="$(profile_suffix "$profile")"

  cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
    -DOPENCC_SOURCE_PACKAGE_PROFILE="$profile" \
    -DCPACK_SOURCE_ARCHIVE_TIMESTAMP=0
  (cd "$BUILD_DIR" && cpack --config CPackSourceConfig.cmake -G TGZ)

  local cpack_tarball base tmpdir tar_bin
  tar_bin="$(gnu_tar)"
  cpack_tarball="$(ls "$BUILD_DIR"/opencc-*-"$suffix".tar.gz | head -n 1)"
  base="$(basename "$cpack_tarball" .tar.gz)"
  tmpdir="$(mktemp -d)"
  trap 'rm -rf "$tmpdir"' RETURN

  tar xzf "$cpack_tarball" -C "$tmpdir"
  mkdir -p "$DIST_DIR"
  "$tar_bin" cf - -C "$tmpdir" --sort=name --owner=root --group=root --mtime=@0 "$base" |
    gzip -9n >"$DIST_DIR/${base}.tar.gz"
  "$tar_bin" cf - -C "$tmpdir" --sort=name --owner=root --group=root --mtime=@0 "$base" |
    xz -9 >"$DIST_DIR/${base}.tar.xz"
  ls -l "$DIST_DIR"
}

smoke_test() {
  local profile="$1"
  local tarball="${2:-}"
  [ -n "$tarball" ] || tarball="$(find_tarball "$profile")"

  local workdir src
  workdir="${WORK_DIR:-$(mktemp -d)}"
  mkdir -p "$workdir"
  tar xzf "$tarball" -C "$workdir"
  src="$(echo "$workdir"/opencc-*-"$(profile_suffix "$profile")")"
  echo "Smoke-testing $tarball in $src"

  # CMake: configure and build the core library, CLI tools, and Jieba plugin.
  cmake -S "$src" -B "$src/build" -DCMAKE_BUILD_TYPE=Release -DBUILD_OPENCC_JIEBA_PLUGIN=ON
  cmake --build "$src/build" --target opencc opencc_dict opencc_jieba --parallel

  # Bazel: catches a trimmed package that dropped a file the Bazel module or
  # BUILD graph needs. Explicit target patterns are used instead of //...
  # because the vendored deps/googletest-1.15.0 BUILD files do not load under
  # current Bazel versions.
  if [ "$profile" = "bazel" ]; then
    (cd "$src" && bazel build //:opencc //src/tools/... //plugins/... //node/...)
    (cd "$src" && bazel test --skip_incompatible_explicit_targets \
      //src/... //data/... //test/... //python/... //plugins/... //node/test:node_test)
  else
    (cd "$src" && bazel build \
      //src/tools:dict_converter //data/dictionary:binary_dictionaries //plugins/jieba:opencc-jieba)
  fi
}

command="${1:-}"
profile="${2:-}"
case "$command" in
build)
  [ -n "$profile" ] || { usage >&2; exit 1; }
  build_package "$profile"
  ;;
smoke-test)
  [ -n "$profile" ] || { usage >&2; exit 1; }
  smoke_test "$profile" "${3:-}"
  ;;
all)
  [ -n "$profile" ] || { usage >&2; exit 1; }
  build_package "$profile"
  smoke_test "$profile"
  ;;
*)
  usage >&2
  exit 1
  ;;
esac
