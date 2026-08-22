# Debian Packaging Proposals

This directory carries upstream OpenCC's proposed Debian packaging. It is
maintained by upstream as a set of suggestions to the Debian `opencc`
maintainers; Debian is under no obligation to adopt any of it, and the
authoritative packaging is whatever Debian actually ships.

## 1. Baseline

To keep proposals reviewable as deltas, `debian/` here is periodically
synced to the packaging Debian currently ships:

- Upstream: BYVoid/OpenCC `ver.1.4.2`
- Debian source package: `opencc 1.4.1+ds1-8` (unstable)
- Synced from packaging-repository commit `e4d7c5e8` (2026-08-21,
  "Override arch-dep-package-has-big-usr-share for opencc-jieba")
- Debian packaging repository:
  https://salsa.debian.org/debian/opencc
- Patch reference index:
  https://udd.debian.org/patches.cgi?src=opencc

As of this sync the only upstream deltas are release bookkeeping for
1.4.2: a `1.4.2-1` `UNRELEASED` changelog entry (`release-deb.yml` stamps
the built artifacts with `dpkg-parsechangelog`, which would otherwise
still report `1.4.1+ds1-8`) and the removal of the now-inapplicable
`0010-fix-git-version-fallback.patch`. Otherwise `debian/` is a verbatim
copy of the Debian packaging repository. The two proposals previously
carried here have been adopted, with implementation changes — and one of
them has since been partly reverted:

- **Jieba plugin packaging** (Debian bug
  [#1141451](https://bugs.debian.org/1141451)): adopted in
  `1.4.1+ds1-5`, but split into two binary packages instead of the
  single `Architecture: any` package upstream proposed —
  `opencc-jieba` (arch:any, the plugin) and `opencc-jieba-data`
  (arch:all, the merged Jieba dictionary and `*_jieba.json` configs).
  The bundled `cppjieba` question is documented in
  `debian/README.source` rather than treated as a blocker.
- **`*.ocd2` dictionaries in `libopencc-data`**: adopted in
  `1.4.1+ds1-4`, then **reverted in `1.4.1+ds1-8`** — this upstream
  proposal was wrong. `.ocd2` files are marisa-trie images written in
  host byte order, so shipping them from an `Architecture: all` package
  gave every architecture the little-endian copies built on the arch:all
  buildd, and opencc failed at runtime on s390x with `size > avail_`
  (Debian bug [#1145010](https://bugs.debian.org/1145010), caught by the
  s390x autopkgtest re-enabled in `-7`). `usr/share/opencc/*.ocd2` is
  back in the arch:any `libopencc1.4` and
  `jieba_dict/jieba_merged.ocd2` back in the arch:any `opencc-jieba`,
  with matching `Breaks`/`Replaces` against the `-8~` versions of the
  data packages. What survives of the proposal is that
  `debian/libopencc-data.install` lists the non-Jieba configurations
  explicitly instead of globbing `usr/share/opencc/*.json`, which would
  otherwise also capture the `*_jieba.json` files.

## 2. Patch set

Current quilt patch set (see `debian/patches/series`):

- 0003-no-remote-images-when-reading-docs-on-disk.patch
- 0004-Use-system-googletest.patch
- 0005-Disable-build-in-setup.py.patch
- 0009-setup.py-Handle-python-binding-instead-of-cmake.patch
- 0011-drop-python-cli-entry-point.patch
- 0012-fix-legacy-dict-detection-on-big-endian.patch

Patch numbering reflects historical Debian quilt ordering and is not
contiguous. 0010 was dropped for the 1.4.2 release: upstream now sets
`_OPENCC_FALLBACK_REVISION` to the release revision in
`cmake/GitVersion.cmake`, so the patch no longer applies. 0012 is
`Applied-Upstream` (commit `8aeb5e69`) and is contained in 1.4.2; it can
be dropped once Debian moves to that release.

## 3. Binary packages (in this tree)

- `opencc` — command line tools
- `libopencc1.4` — runtime shared library and the `*.ocd2` dictionaries
  (tracks `OPENCC_ABI_VERSION`; renamed from `libopencc1.3`, with a
  regenerated `.symbols` file)
- `libopencc-dev` — headers, pkg-config, CMake package config
- `libopencc-data` — JSON configurations only (arch-independent)
- `libopencc-doc` — Doxygen HTML API docs
- `python3-opencc` — Python bindings built via pybuild/pyproject
- `opencc-jieba` — Jieba segmentation plugin (`libopencc-jieba.so`) and
  the merged `jieba_merged.ocd2` dictionary
- `opencc-jieba-data` — plain-text `jieba_dict/*.utf8` sources and the
  `*_jieba.json` configurations (arch-independent)

## 4. Notes on the current packaging

The runtime package name tracks `OPENCC_ABI_VERSION` from
`CMakeLists.txt`, so an ABI-incompatible change to the public C++
interface means both a version bump upstream and a rename of
`libopencc1.4` here. `libopencc-dev` depends on `libopencc-data`, not
just on `libopencc1.4`, because the installed `OpenCCConfig.cmake` bakes
in `DIR_SHARE_OPENCC`; it is deliberately *not* `Multi-Arch: same`,
since those generated CMake files embed the architecture triplet and so
are not byte-identical across architectures.

`libopencc1.4` must not be `Multi-Arch: same` either, even though the
multiarch hinter suggests it: since `1.4.1+ds1-8` it ships the `*.ocd2`
dictionaries, whose marisa-trie images are written in host byte order
and therefore differ between architectures of opposite endianness. The
same constraint means `*.ocd2` must never be shipped from an
`Architecture: all` package — see `debian/README.source` and section 1.
`opencc-jieba` carries an `arch-dep-package-has-big-usr-share` lintian
override for the resulting `jieba_merged.ocd2`.

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
`*_jieba.json` conversions). It immediately earned its keep: the s390x
run is what surfaced the arch:all `*.ocd2` breakage fixed in `-8`. The forked test reads configurations from
`/usr/share/opencc` and passes `--include-tofu-risk-dictionaries`, so
changes to `test/testcases/testcases.json` or to the CLI's config
resolution can break it — see `debian/tests/README.md`.

## 5. Refreshing the baseline

Copy the `debian/` directory from the salsa repository checkout over
`packaging/debian/debian/`, re-apply any upstream proposal deltas that
Debian has not adopted (currently only the release bookkeeping described
in section 1), and update the baseline section above.
