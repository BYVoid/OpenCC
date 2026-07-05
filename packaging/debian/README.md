# Debian Packaging Patch Mirror

This directory provides a mirrored view of Debian packaging deltas for OpenCC,
intended for upstream review.

It is not the authoritative Debian packaging source.

## 1. Baseline

This patch set is based on:

- Upstream: BYVoid/OpenCC `ver.1.4.0`
- Debian source package: `opencc 1.3.1+ds1-3`
- Debian packaging repository:
  https://salsa.debian.org/debian/opencc
- Patch reference index:
  https://udd.debian.org/patches.cgi?src=opencc

## 2. Patch set

Current quilt patch set:

- 0003
- 0004
- 0005
- 0009

This is the complete active set.

Patch numbering reflects historical Debian quilt ordering and is not contiguous.

## 3. Patch status against upstream 1.4.0

All patches have been re-based against upstream `1.4.0` and verified to apply cleanly:

- `patch -p1 --fuzz=0` compatibility
- tested in clean upstream checkout

Where upstream changes affected context, patches were adjusted to match
current structure while preserving original intent.

Original Debian metadata (where available) is retained in patch headers.

## 4. Design intent

This mirror is provided to:

- expose Debian packaging deltas for upstream review
- identify patches eligible for replacement by upstream build options
- reduce divergence between Debian packaging and upstream defaults

Several patches correspond to functionality now covered by upstream CMake options
(e.g. `USE_SYSTEM_*`). See `SPLIT_PROPOSAL.md`.

## 5. Packaging layout (non-authoritative)

The full Debian packaging layout is located at:

```
packaging/debian/debian/
```

This directory can be overlaid onto a clean upstream checkout to reproduce
the Debian build using `dpkg-buildpackage`.

See:

```
packaging/debian/debian/README.source
```

for build instructions.

## 6. Scope

This directory is a review mirror only and does not represent authoritative Debian packaging.
