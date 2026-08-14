# Debian Packaging Proposals

This directory carries upstream OpenCC's proposed Debian packaging. It is
maintained by upstream as a set of suggestions to the Debian `opencc`
maintainers; Debian is under no obligation to adopt any of it, and the
authoritative packaging is whatever Debian actually ships.

## 1. Baseline

To keep proposals reviewable as deltas, `debian/` here is periodically
synced to the packaging Debian currently ships:

- Upstream: BYVoid/OpenCC `ver.1.4.1`
- Debian source package: `opencc 1.4.1+ds1-7` (unstable)
- Debian packaging repository:
  https://salsa.debian.org/debian/opencc
- Patch reference index:
  https://udd.debian.org/patches.cgi?src=opencc

As of this sync there are **no outstanding upstream deltas**: `debian/`
is a verbatim copy of the Debian packaging repository. The two proposals
previously carried here have been adopted, with implementation changes:

- **Jieba plugin packaging** (Debian bug
  [#1141451](https://bugs.debian.org/1141451)): adopted in
  `1.4.1+ds1-5`, but split into two binary packages instead of the
  single `Architecture: any` package upstream proposed —
  `opencc-jieba` (arch:any, the plugin) and `opencc-jieba-data`
  (arch:all, the merged Jieba dictionary and `*_jieba.json` configs).
  The bundled `cppjieba` question is documented in
  `debian/README.source` rather than treated as a blocker.
- **`*.ocd2` dictionaries in `libopencc-data`**: adopted in
  `1.4.1+ds1-4`. `debian/libopencc-data.install` now lists the non-Jieba
  configurations explicitly instead of globbing `usr/share/opencc/*.json`,
  which would otherwise also capture the `*_jieba.json` files.

## 2. Patch set

Current quilt patch set (see `debian/patches/series`):

- 0003-no-remote-images-when-reading-docs-on-disk.patch
- 0004-Use-system-googletest.patch
- 0005-Disable-build-in-setup.py.patch
- 0009-setup.py-Handle-python-binding-instead-of-cmake.patch
- 0010-fix-git-version-fallback.patch
- 0011-drop-python-cli-entry-point.patch
- 0012-fix-legacy-dict-detection-on-big-endian.patch

Patch numbering reflects historical Debian quilt ordering and is not
contiguous. 0012 is `Applied-Upstream` (commit `8aeb5e69`) and can be
dropped once Debian moves to a release containing it.

## 3. Binary packages (in this tree)

- `opencc` — command line tools
- `libopencc1.4` — runtime shared library (tracks `OPENCC_ABI_VERSION`;
  renamed from `libopencc1.3`, with a regenerated `.symbols` file)
- `libopencc-dev` — headers, pkg-config, CMake package config
- `libopencc-data` — JSON configs and `*.ocd2` dictionaries
  (arch-independent)
- `libopencc-doc` — Doxygen HTML API docs
- `python3-opencc` — Python bindings built via pybuild/pyproject
- `opencc-jieba` — Jieba segmentation plugin (`libopencc-jieba.so`)
- `opencc-jieba-data` — merged Jieba dictionary and `*_jieba.json`
  configurations (arch-independent)

## 4. Notes on the current packaging

The runtime package name tracks `OPENCC_ABI_VERSION` from
`CMakeLists.txt`, so an ABI-incompatible change to the public C++
interface means both a version bump upstream and a rename of
`libopencc1.4` here. `libopencc-dev` depends on `libopencc-data`, not
just on `libopencc1.4`, because the installed `OpenCCConfig.cmake` bakes
in `DIR_SHARE_OPENCC`; it is deliberately *not* `Multi-Arch: same`,
since those generated CMake files embed the architecture triplet and so
are not byte-identical across architectures.

Several historical Debian patches correspond to functionality now covered
by upstream CMake options (e.g. `USE_SYSTEM_*`); `debian/rules` builds
with those options against Debian-packaged libraries. The exception is
`USE_SYSTEM_DARTS`, which must stay `OFF`: `src/DartsDict.cpp` calls
`Darts::DoubleArray::validate()`, which only exists in the vendored
`deps/darts-clone-0.32h` header (darts-clone v0.32h plus the
google/sentencepiece hardening, added upstream in 1.4.0). Debian has no
darts-clone package at all — its `darts` package is a different upstream
(Taku Kudo's Darts) — so building against a system header is not
possible, and `deps/darts-clone-*` is no longer stripped from the +ds1
tarball. `plugins/jieba/deps/cppjieba` stays bundled for the reasons
spelled out in `debian/README.source`.

Autopkgtest was re-enabled in `1.4.1+ds1-7`: `debian/tests/control` now
runs `cli-smoke` (installed CLI checks), `integration` (the forked
`CommandLineConvertTest` built against `libopencc-dev` and run over
`test/testcases/testcases.json`), and `jieba-plugin` (plugin loading and
`*_jieba.json` conversions). The forked test reads configurations from
`/usr/share/opencc` and passes `--include-tofu-risk-dictionaries`, so
changes to `test/testcases/testcases.json` or to the CLI's config
resolution can break it — see `debian/tests/README.md`.

## 5. Refreshing the baseline

Copy the `debian/` directory from the salsa repository checkout over
`packaging/debian/debian/`, re-apply any upstream proposal deltas that
Debian has not adopted (currently none), and update the baseline section
above.
