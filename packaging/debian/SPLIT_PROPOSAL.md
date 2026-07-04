# Proposed Debian binary package split for opencc 1.4.x

This is a proposal from upstream, not a Debian upload. It was produced by
overlaying the top-level `debian/` directory onto a clean checkout of
upstream `BYVoid/OpenCC` at `master` (1.4.0) and actually running
`dpkg-buildpackage -us -uc -b` there, on Ubuntu 24.04 with debhelper
13.14. All six binary packages built, `dh_missing --fail-missing` found
nothing unclaimed, `ctest` passed 19/19, and `lintian --pedantic`
reported no errors (only expected first-upload and missing-manpage
warnings). See `debian/README.source` for the gaps this recreation still
has before it would be upload-ready.

## Why keep the existing 5-package split

Debian's `opencc` source package already ships five binary packages
(`opencc`, `libopencc1.1`, `libopencc-dev`, `libopencc-data`,
`libopencc-doc`); see `https://tracker.debian.org/pkg/opencc`. That split
already follows Debian's standard shared-library layout, so this proposal
keeps it as-is rather than inventing a new scheme, with one change: the
runtime package name tracks `OPENCC_ABI_VERSION` from `CMakeLists.txt`,
which is `1.4` as of this release (`libopencc1.1` -> `libopencc1.4`).
`OPENCC_ABI_VERSION` (and therefore this package name) must be bumped
again any time a change modifies the public C++ interface in an
ABI-incompatible way — this includes changes to installed headers under
`include/opencc/`, exported virtual interfaces, class layout,
constructor/destructor signatures, inline public methods, or any other
public symbol that downstream C++ consumers may have compiled against.

| Package | Arch | Section | Contents |
|---|---|---|---|
| `opencc` | any | utils | `opencc`, `opencc_dict`, `opencc_phrase_extract` CLI binaries |
| `libopencc1.4` | any | libs | `libopencc.so.1.4*` runtime only |
| `libopencc-dev` | any | libdevel | headers, unversioned `.so` symlink, `opencc.pc`, CMake package config |
| `libopencc-data` | all, Multi-Arch: foreign | libs | `*.ocd2` dictionaries and the non-Jieba `*.json` configs |
| `libopencc-doc` | all | doc | Doxygen HTML API docs |

Rationale, matching what the install-rule inventory in CMake already
implies (see the file-by-file mapping validated in
`packaging/debian/debian/*.install`):

- `libopencc-data` is `Architecture: all` and `Multi-Arch: foreign`
  because dictionaries and JSON configs are architecture-independent;
  splitting them out means a multi-arch install (e.g. `:i386` alongside
  `:amd64`) does not duplicate ~5 MB of dictionary data per architecture.
- `libopencc-dev` depends on `libopencc-data` (not just `libopencc1.4`)
  because `OpenCCConfig.cmake` bakes in `DIR_SHARE_OPENCC`, so a
  from-source consumer of the CMake package config needs the data
  present at configure/link time too, matching upstream's own
  `PATH_VARS DIR_INCLUDE DIR_PLUGIN DIR_SHARE_OPENCC` in `CMakeLists.txt`.
- Not proposing `Multi-Arch: same` for `libopencc-dev`: the installed
  `OpenCCConfig.cmake` / `OpenCCTargets.cmake` files embed the
  architecture triplet in `DIR_LIBRARY`-derived paths, so they are not
  byte-identical across architectures. Marking it `Multi-Arch: same`
  would violate Debian Policy's co-installability requirement unless
  those generated files are made architecture-independent first
  (out of scope here).

## Proposed new package: `opencc-jieba`

Debian does not currently package the Jieba segmentation plugin
(`plugins/jieba`) at all — it isn't listed in the existing source
package. This proposal adds it as a sixth binary package, matching the
name already used by this repository's own `scripts/build-deb-packages.sh`
release artifact for continuity:

| Package | Arch | Section | Contents |
|---|---|---|---|
| `opencc-jieba` | any | utils | `libopencc-jieba.so` plugin, merged Jieba `.ocd2`, `*_jieba.json` configs |

Depends: `opencc (= ${binary:Version})`, `libopencc-data (>= ${source:Version})`.

`plugins/jieba/CMakeLists.txt` also builds `opencc_jieba_dict_build_tool`,
the offline tool that merges the bundled Jieba dictionaries into
`jieba_dict/jieba_merged.ocd2` at build time. Upstream does not
`install()` this tool, so it is a build dependency only and is not
shipped in `opencc-jieba` (or any package) here either.

Known blocker for a real Debian upload: `plugins/jieba/deps/cppjieba` is
a header-only C++ library with **no existing Debian package** to build
against, so `opencc-jieba` would have to bundle it (see the
`debian/copyright` entry for the license and rationale). Two paths
forward, in order of preference:

1. Someone packages `cppjieba` for Debian first (it is small and
   header-only, so this is plausible), and `opencc-jieba`'s
   build-dependency becomes `libcppjieba-dev` instead of a bundled copy.
2. `opencc-jieba` ships with the bundled copy and goes through Debian's
   embedded-code-copy review during upload (`Files-Excluded` does not
   apply here since there is no external package to fall back to).

## What the `debian/rules` in this proposal actually builds

`-DUSE_SYSTEM_MARISA=ON -DUSE_SYSTEM_RAPIDJSON=ON -DUSE_SYSTEM_TCLAP=ON
-DUSE_SYSTEM_GTEST=ON -DBUILD_PYTHON=OFF -DBUILD_DOCUMENTATION=ON
-DBUILD_OPENCC_JIEBA_PLUGIN=ON`, i.e. every bundled `deps/` copy that has
an upstream `USE_SYSTEM_*` toggle is switched off in favor of the
Debian-packaged library (`libmarisa-dev`, `rapidjson-dev`,
`libtclap-dev`, `googletest`/`libgtest-dev`). `deps/darts-clone-0.32h`
has no such toggle (Darts support is always compiled in; there is no
`USE_SYSTEM_DARTS` option) and stays bundled — same situation as
`cppjieba` above, but lower risk since darts-clone is tiny and already
used this way by the real Debian package's patch queue history.

Python bindings (`setup.py`, `python/opencc`) are intentionally left out
of this proposal: Debian does not currently ship a `python3-opencc`
binary from the `opencc` source package either (the module is
PyPI-only), and `packaging/debian/patches/0005` and `0009` exist to keep
`setup.py`-based builds (used by pip/PyPI, not by this `debian/rules`)
working when CMake's own build is disabled — see those patches'
descriptions.

## Suggested next steps for whoever picks this up on the Debian side

1. Decide on the `cppjieba` bundling question above before adding
   `opencc-jieba` to an actual upload.
2. Add a `debian/libopencc1.4.symbols` file for finer-grained
   `${shlibs:Depends}` versioning (this recreation relies on the
   auto-generated shlibs file only).
3. Add man pages for `opencc`, `opencc_dict`, and `opencc_phrase_extract`
   (flagged by `lintian --pedantic` as `no-manual-page`; not currently
   shipped anywhere in this repository).
