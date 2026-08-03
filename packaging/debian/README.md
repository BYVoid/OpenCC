# Debian Packaging Proposals

This directory carries upstream OpenCC's proposed Debian packaging. It is
maintained by upstream as a set of suggestions to the Debian `opencc`
maintainers; Debian is under no obligation to adopt any of it, and the
authoritative packaging is whatever Debian actually ships.

## 1. Baseline

To keep proposals reviewable as deltas, `debian/` here is periodically
synced to the packaging Debian currently ships:

- Upstream: BYVoid/OpenCC `ver.1.4.1`
- Debian source package: `opencc 1.4.1+ds1-1~exp1` (experimental)
- Debian packaging repository:
  https://salsa.debian.org/debian/opencc
- Patch reference index:
  https://udd.debian.org/patches.cgi?src=opencc

On top of that baseline, upstream proposals not (yet) adopted by Debian
are applied as deltas. Currently:

- **`opencc-jieba` binary package** (see `SPLIT_PROPOSAL.md`): the
  `debian/control` stanza, `-DBUILD_OPENCC_JIEBA_PLUGIN=ON` in
  `debian/rules`, and `debian/opencc-jieba.install`.
- **`*.ocd2` dictionaries moved into `libopencc-data`**: Debian
  currently ships them in the `Architecture: any` `libopencc1.4`
  package; this tree moves them to the arch-independent data package
  (`libopencc-data.install` / `libopencc1.4.install`) so multi-arch
  installs do not duplicate dictionary data per architecture.

## 2. Patch set

Current quilt patch set (see `debian/patches/series`):

- 0003-no-remote-images-when-reading-docs-on-disk.patch
- 0004-Use-system-googletest.patch
- 0005-Disable-build-in-setup.py.patch
- 0009-setup.py-Handle-python-binding-instead-of-cmake.patch
- 0010-fix-git-version-fallback.patch
- 0011-drop-python-cli-entry-point.patch

Patch numbering reflects historical Debian quilt ordering and is not
contiguous.

## 3. Binary packages (in this tree)

- `opencc` — command line tools
- `libopencc1.4` — runtime shared library (tracks `OPENCC_ABI_VERSION`;
  renamed from `libopencc1.3`, with a regenerated `.symbols` file)
- `libopencc-dev` — headers, pkg-config, CMake package config
- `libopencc-data` — JSON configs and `*.ocd2` dictionaries
  (arch-independent; the `*.ocd2` placement is an upstream proposal —
  Debian currently ships them in `libopencc1.4`)
- `libopencc-doc` — Doxygen HTML API docs
- `python3-opencc` — Python bindings built via pybuild/pyproject
- `opencc-jieba` — Jieba segmentation plugin, merged Jieba dictionary,
  and `*_jieba.json` configs (upstream proposal, not shipped by Debian;
  blocked on the `cppjieba` bundling question — see `SPLIT_PROPOSAL.md`)

## 4. Notes on the current packaging

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
tarball.

## 5. Refreshing the baseline

Copy the `debian/` directory from the salsa repository checkout over
`packaging/debian/debian/`, re-apply the upstream proposal deltas listed
in section 1, and update the baseline section above.
