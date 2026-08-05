#!/usr/bin/env bash
#
# Build Debian binary packages via dpkg-buildpackage, using
# packaging/debian/debian/ as the packaging directory.
#
# Usage:
#   bash scripts/build-deb-packages.sh
#
# Environment variables:
#   DIST_DIR        Directory to write .deb files (default: <repo>/dist)
#   BUILD_MODE      "all"      = arch:any + arch:all packages (default; use for native builds)
#                   "any-only" = arch:any packages only (use for secondary architectures
#                                to avoid duplicating arch:all packages)
#   TARGET_ARCH     Debian architecture to cross-compile for (e.g. arm64).
#                   When set, dpkg-buildpackage -a <arch> is used and DEB_BUILD_OPTIONS
#                   defaults to "noautodbgsym nocheck" (test binaries are compiled for the
#                   target arch and cannot run on the build host).
#   DEB_BUILD_OPTIONS  Forwarded to dpkg-buildpackage.  If unset, defaults to
#                      "noautodbgsym" (native) or "noautodbgsym nocheck" (cross).

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="${DIST_DIR:-$ROOT_DIR/dist}"
BUILD_MODE="${BUILD_MODE:-all}"
TARGET_ARCH="${TARGET_ARCH:-}"

# Patches applied by apply_patches, in application order, for cleanup.
APPLIED_PATCHES=()

# dpkg-buildpackage does not apply debian/patches when building from a
# plain git tree (dpkg-source --before-build is a no-op outside an
# unpacked source package), so apply the quilt series ourselves.  A patch
# whose change is already present in the tree (reverse-applies cleanly,
# e.g. a fix that has since landed upstream) is skipped.
apply_patches() {
  local series="$ROOT_DIR/debian/patches/series"
  [ -f "$series" ] || return 0
  local name
  while IFS= read -r name; do
    case "$name" in ''|'#'*) continue ;; esac
    local file="$ROOT_DIR/debian/patches/$name"
    if patch -p1 -R --dry-run -s -f -d "$ROOT_DIR" < "$file" >/dev/null 2>&1; then
      printf 'Skipping patch already applied upstream: %s\n' "$name"
    else
      printf 'Applying patch: %s\n' "$name"
      patch -p1 --forward --no-backup-if-mismatch -d "$ROOT_DIR" < "$file"
      APPLIED_PATCHES+=("$name")
    fi
  done < "$series"
}

cleanup() {
  local i
  for (( i=${#APPLIED_PATCHES[@]}-1; i>=0; i-- )); do
    patch -p1 -R -s -f --no-backup-if-mismatch -d "$ROOT_DIR" \
      < "$ROOT_DIR/debian/patches/${APPLIED_PATCHES[$i]}" || true
  done
  rm -rf "$ROOT_DIR/debian"
}

main() {
  if [ -e "$ROOT_DIR/debian" ]; then
    printf 'Error: %s/debian already exists. Remove it before running this script.\n' \
      "$ROOT_DIR" >&2
    exit 1
  fi

  cp -r "$ROOT_DIR/packaging/debian/debian" "$ROOT_DIR/debian"
  trap cleanup EXIT

  apply_patches

  local args=(-us -uc)
  [ -n "$TARGET_ARCH" ] && args+=(-a "$TARGET_ARCH")
  case "$BUILD_MODE" in
    all)      args+=(-b) ;;
    any-only) args+=(-B) ;;
    *)
      printf 'Unknown BUILD_MODE: %s (expected "all" or "any-only")\n' \
        "$BUILD_MODE" >&2
      exit 1
      ;;
  esac

  if [ -z "${DEB_BUILD_OPTIONS:-}" ]; then
    if [ -n "$TARGET_ARCH" ]; then
      export DEB_BUILD_OPTIONS="noautodbgsym nocheck"
    else
      export DEB_BUILD_OPTIONS="noautodbgsym"
    fi
  fi

  # Use a temp file as a timestamp marker so we can identify freshly produced
  # .deb files in the parent directory without accidentally grabbing stale ones.
  local marker
  marker="$(mktemp)"

  (cd "$ROOT_DIR" && dpkg-buildpackage "${args[@]}")

  # dpkg-buildpackage writes output into the parent of the source directory.
  local out_dir
  out_dir="$(dirname "$ROOT_DIR")"
  mkdir -p "$DIST_DIR"
  find "$out_dir" -maxdepth 1 -name '*.deb' -newer "$marker" \
    -exec mv {} "$DIST_DIR/" \;
  find "$out_dir" -maxdepth 1 \( -name '*.changes' -o -name '*.buildinfo' \) \
    -newer "$marker" -delete
  rm -f "$marker"

  printf '\nPackages written to %s:\n' "$DIST_DIR"
  ls -1 "$DIST_DIR"/*.deb
}

main "$@"
