# FreeBSD Port (Reference Copy)

This directory carries a reference copy of the FreeBSD ports-tree entry
for OpenCC, `chinese/opencc`. The authoritative port lives in the FreeBSD
ports tree and is maintained there by its port maintainer; this copy
exists so upstream can review the packaging, stage proposed changes for
new releases, and hand them to the maintainer (or submit them via
FreeBSD Bugzilla). It is not independently buildable — the `Makefile`
depends on the ports framework (`Mk/`) infrastructure.

## 1. Baseline

- Upstream: BYVoid/OpenCC `ver.1.4.1`
- Ports tree path: `chinese/opencc`
  https://github.com/freebsd/freebsd-ports/tree/main/chinese/opencc
- Synced from ports-tree commit `584636d93a32` (2026-07-16,
  "chinese/opencc: Upgrade from 1.3.1 to 1.4.1.")
- FreshPorts page: https://www.freshports.org/chinese/opencc/

No upstream deltas are currently applied on top of the baseline; this is
a verbatim copy of what the ports tree ships.

## 2. Contents

- `Makefile` — port definition. Builds with CMake against the GitHub
  release tarball (`DISTVERSIONPREFIX=ver.`), enables GTest-based tests,
  and offers a `JIEBA` option (default on) mapping to
  `BUILD_OPENCC_JIEBA_PLUGIN`.
- `distinfo` — checksum and size of the release tarball.
- `pkg-descr` / `pkg-plist` — package description and installed-file
  manifest (with `%%JIEBA%%` conditionals from `OPTIONS_SUB`).
- `files/patch-CMakeLists.txt` — installs `opencc.pc` into FreeBSD's
  `libdata/pkgconfig` instead of `${DIR_LIBRARY}/pkgconfig`.
- `files/patch-cmake_GitVersion.cmake` — bumps the Git-less fallback
  version to 1.4.1 for the release tarball; already fixed upstream in
  commit `c23e743702f4`, so this patch should drop out at the next
  version bump.

## 3. Refreshing the baseline

Copy the current `chinese/opencc/` directory from the ports tree over
this one, re-apply any upstream proposal deltas listed in section 1, and
update the synced-commit line above. When preparing a version bump
proposal, regenerate `distinfo` (`make makesum` in a ports checkout),
review `pkg-plist` against the new install manifest, and drop any
`files/` patches that upstream has absorbed.
