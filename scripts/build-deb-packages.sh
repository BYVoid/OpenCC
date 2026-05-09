#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="${DIST_DIR:-$ROOT_DIR/dist}"
WORK_DIR="${WORK_DIR:-$ROOT_DIR/build-deb}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr}"
MAINTAINER="${MAINTAINER:-OpenCC Contributors <opencc@users.noreply.github.com>}"
TARGET_ARCH="${TARGET_ARCH:-$(dpkg --print-architecture)}"
CMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE:-}"
HOST_CORE_DICT_SOURCE_DIR="${HOST_CORE_DICT_SOURCE_DIR:-}"
HOST_JIEBA_MERGED_DICT_SOURCE="${HOST_JIEBA_MERGED_DICT_SOURCE:-}"

build_host_tools_and_resources() {
  local core_build_dir="$1"
  local plugin_build_dir="$2"
  local host_pkgroot="$3"

  cmake -S "$ROOT_DIR" -B "$core_build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_OPENCC_JIEBA_PLUGIN=OFF \
    -DUSE_SYSTEM_MARISA=OFF
  cmake --build "$core_build_dir"
  DESTDIR="$host_pkgroot" cmake --install "$core_build_dir"

  local host_opencc_package_dir
  host_opencc_package_dir="$(find_opencc_package_dir "$host_pkgroot")"

  cmake -S "$ROOT_DIR/plugins/jieba" -B "$plugin_build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DOpenCC_DIR="$host_opencc_package_dir"
  cmake --build "$plugin_build_dir"
}

compute_version() {
  local raw_version
  raw_version="${1:-$(bash "$ROOT_DIR/scripts/compute-version.sh")}"
  raw_version="${raw_version#ver.}"
  raw_version="${raw_version#v}"
  printf '%s\n' "$raw_version"
}

write_control_file() {
  local path="$1"
  local package_name="$2"
  local version="$3"
  local architecture="$4"
  local depends="$5"
  local description="$6"
  local installed_size

  installed_size="$(du -sk "$(dirname "$path")/.." | awk '{print $1}')"

  cat >"$path" <<EOF
Package: ${package_name}
Version: ${version}
Section: utils
Priority: optional
Architecture: ${architecture}
Maintainer: ${MAINTAINER}
Installed-Size: ${installed_size}
Depends: ${depends}
Homepage: https://github.com/BYVoid/OpenCC
Description: ${description}
EOF
}

write_doc_files() {
  local pkgroot="$1"
  local package_name="$2"

  mkdir -p "$pkgroot${INSTALL_PREFIX}/share/doc/${package_name}"
  cp "$ROOT_DIR/LICENSE" "$pkgroot${INSTALL_PREFIX}/share/doc/${package_name}/copyright"
  cp "$ROOT_DIR/README.md" "$pkgroot${INSTALL_PREFIX}/share/doc/${package_name}/README.md"
}

write_debian_control_stub() {
  local control_dir="$1"
  local package_name="$2"

  mkdir -p "$control_dir"
  cat >"$control_dir/control" <<EOF
Source: opencc-local
Section: utils
Priority: optional
Maintainer: ${MAINTAINER}
Standards-Version: 4.6.2

Package: ${package_name}
Architecture: any
Description: temporary package metadata for dpkg-shlibdeps
EOF
}

find_opencc_package_dir() {
  local pkgroot="$1"
  local config_path

  config_path="$(find "$pkgroot${INSTALL_PREFIX}" -path '*/cmake/opencc/OpenCCConfig.cmake' -print -quit)"
  if [ -z "$config_path" ]; then
    printf 'Unable to locate OpenCCConfig.cmake under %s\n' "$pkgroot${INSTALL_PREFIX}" >&2
    exit 1
  fi

  dirname "$config_path"
}

find_jieba_plugin_library() {
  local pkgroot="$1"
  local plugin_path

  plugin_path="$(find "$pkgroot${INSTALL_PREFIX}" -path '*/opencc/plugins/libopencc-jieba.so' -print -quit)"
  if [ -z "$plugin_path" ]; then
    printf 'Unable to locate libopencc-jieba.so under %s\n' "$pkgroot${INSTALL_PREFIX}" >&2
    exit 1
  fi

  printf '%s\n' "$plugin_path"
}

collect_shlib_deps() {
  local package_name="$1"
  shift
  local target_arch="$1"
  shift

  local files=("$@")
  local output
  local control_dir

  control_dir="$(mktemp -d)"
  trap 'rm -rf "$control_dir"' RETURN
  write_debian_control_stub "$control_dir/debian" "$package_name"

  output="$(cd "$control_dir" && DEB_HOST_ARCH="$target_arch" dpkg-shlibdeps -O "${files[@]}")"
  output="${output#shlibs:Depends=}"
  printf '%s\n' "$output"
}

copy_generated_core_dicts() {
  local source_build_dir="$1"
  local dest_build_dir="$2"
  local source_dict_dir="$source_build_dir"

  if [ -d "$source_build_dir/data" ]; then
    source_dict_dir="$source_build_dir/data"
  fi

  mkdir -p "$dest_build_dir/data"
  find "$source_dict_dir" -maxdepth 1 -name '*.ocd2' -exec cp {} "$dest_build_dir/data/" \;
}

copy_generated_jieba_dict() {
  local merged_dict_path="$1"
  local pkgroot="$2"

  mkdir -p "$pkgroot${INSTALL_PREFIX}/share/opencc/jieba_dict"
  cp "$merged_dict_path" "$pkgroot${INSTALL_PREFIX}/share/opencc/jieba_dict/jieba_merged.ocd2"
}

have_prebuilt_host_resources() {
  [ -n "$HOST_CORE_DICT_SOURCE_DIR" ] &&
  [ -d "$HOST_CORE_DICT_SOURCE_DIR" ] &&
  [ -n "$HOST_JIEBA_MERGED_DICT_SOURCE" ] &&
  [ -f "$HOST_JIEBA_MERGED_DICT_SOURCE" ]
}

main() {
  local version architecture host_arch
  version="$(compute_version "${1:-}")"
  architecture="$TARGET_ARCH"
  host_arch="$(dpkg --print-architecture)"

  local core_build_dir="$WORK_DIR/core"
  local plugin_build_dir="$WORK_DIR/jieba"
  local core_pkgroot="$WORK_DIR/pkgroot-opencc"
  local plugin_pkgroot="$WORK_DIR/pkgroot-opencc-jieba"
  local host_core_build_dir="$WORK_DIR/host-core"
  local host_plugin_build_dir="$WORK_DIR/host-jieba"
  local host_pkgroot="$WORK_DIR/host-pkgroot-opencc"
  local cmake_cross_args=()
  local is_cross_build=FALSE

  if [ "$architecture" != "$host_arch" ]; then
    is_cross_build=TRUE
    if [ -z "$CMAKE_TOOLCHAIN_FILE" ]; then
      printf 'Cross-building for %s requires CMAKE_TOOLCHAIN_FILE to be set.\n' "$architecture" >&2
      exit 1
    fi
    cmake_cross_args+=(-DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE")
  fi

  rm -rf "$WORK_DIR" "$DIST_DIR"
  mkdir -p "$DIST_DIR" "$core_pkgroot/DEBIAN" "$plugin_pkgroot/DEBIAN"

  if [ "$is_cross_build" = TRUE ] && ! have_prebuilt_host_resources; then
    build_host_tools_and_resources "$host_core_build_dir" "$host_plugin_build_dir" "$host_pkgroot"
  fi

  cmake -S "$ROOT_DIR" -B "$core_build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_OPENCC_JIEBA_PLUGIN=OFF \
    -DUSE_SYSTEM_MARISA=OFF \
    "${cmake_cross_args[@]}"
  if [ "$is_cross_build" = TRUE ]; then
    cmake --build "$core_build_dir" --target libopencc opencc opencc_dict opencc_phrase_extract
    if have_prebuilt_host_resources; then
      copy_generated_core_dicts "$HOST_CORE_DICT_SOURCE_DIR" "$core_build_dir"
    else
      copy_generated_core_dicts "$host_core_build_dir" "$core_build_dir"
    fi
  else
    cmake --build "$core_build_dir"
  fi
  DESTDIR="$core_pkgroot" cmake --install "$core_build_dir"
  write_doc_files "$core_pkgroot" opencc

  local core_depends
  core_depends="$(collect_shlib_deps \
    "opencc" \
    "$architecture" \
    "$core_pkgroot${INSTALL_PREFIX}/bin/opencc" \
    "$core_pkgroot${INSTALL_PREFIX}/bin/opencc_dict" \
    "$core_pkgroot${INSTALL_PREFIX}/bin/opencc_phrase_extract")"
  write_control_file \
    "$core_pkgroot/DEBIAN/control" \
    "opencc" \
    "$version" \
    "$architecture" \
    "$core_depends" \
    "Open Chinese Convert command line tools, dictionaries, and development files"

  local opencc_package_dir
  opencc_package_dir="$(find_opencc_package_dir "$core_pkgroot")"

  cmake -S "$ROOT_DIR/plugins/jieba" -B "$plugin_build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DOpenCC_DIR="$opencc_package_dir" \
    "${cmake_cross_args[@]}"
  if [ "$is_cross_build" = TRUE ]; then
    cmake --build "$plugin_build_dir" --target opencc_jieba cppjieba_dict
  else
    cmake --build "$plugin_build_dir"
  fi
  DESTDIR="$plugin_pkgroot" cmake --install "$plugin_build_dir"
  if [ "$is_cross_build" = TRUE ]; then
    if have_prebuilt_host_resources; then
      copy_generated_jieba_dict "$HOST_JIEBA_MERGED_DICT_SOURCE" "$plugin_pkgroot"
    else
      copy_generated_jieba_dict \
        "$host_plugin_build_dir/jieba_dict/jieba_merged.ocd2" \
        "$plugin_pkgroot"
    fi
  fi
  write_doc_files "$plugin_pkgroot" opencc-jieba

  local jieba_plugin_library
  jieba_plugin_library="$(find_jieba_plugin_library "$plugin_pkgroot")"
  local plugin_depends
  plugin_depends="$(collect_shlib_deps \
    "opencc-jieba" \
    "$architecture" \
    "$jieba_plugin_library")"
  plugin_depends="opencc (= ${version}), ${plugin_depends}"
  write_control_file \
    "$plugin_pkgroot/DEBIAN/control" \
    "opencc-jieba" \
    "$version" \
    "$architecture" \
    "$plugin_depends" \
    "Jieba segmentation plugin and dictionaries for OpenCC"

  dpkg-deb --build --root-owner-group "$core_pkgroot" \
    "$DIST_DIR/opencc_${version}_${architecture}.deb"
  dpkg-deb --build --root-owner-group "$plugin_pkgroot" \
    "$DIST_DIR/opencc-jieba_${version}_${architecture}.deb"

  (
    cd "$DIST_DIR"
    sha256sum ./*.deb >SHA256SUMS
  )

  printf 'Built packages:\n'
  printf '  %s\n' "$DIST_DIR/opencc_${version}_${architecture}.deb"
  printf '  %s\n' "$DIST_DIR/opencc-jieba_${version}_${architecture}.deb"
}

main "$@"
