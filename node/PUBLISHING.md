# Publishing Guide

This document describes how to prepare and publish the OpenCC Node.js package
family.

The current npm layout is split by platform:

- `opencc` contains the JavaScript API, CLI, TypeScript declarations, and
  runtime dictionary/config assets.
- `@opencc/opencc-<platform>-<arch>` contains exactly one native
  `opencc.node` binary for one platform.

The native addon comes exclusively from the scoped binary packages. There is
no source-build fallback: on platforms without a scoped binary package,
`npm install` succeeds but requiring `opencc` fails at load time.

## Package names

The main package declares these scoped packages as optional dependencies:

```text
opencc
@opencc/opencc-darwin-arm64
@opencc/opencc-linux-arm64
@opencc/opencc-linux-x64
@opencc/opencc-win32-x64
```

The main package can either point at newly built scoped binary package versions
or reuse already published compatible binary package versions. For a full native
binary refresh, publish all scoped packages at the same version as `opencc`.

## Prerequisites

- npm account access to publish `opencc`.
- npm organization access to publish public packages under `@opencc`.
- Node.js, npm, Python, Bazel (bazelisk), and a native C++ build toolchain
  for the target platform.
- GitHub Actions or another set of machines covering each supported target
  platform.

At the moment, maintainers are assumed to run the full multi-platform release
process through GitHub Actions. A local machine can still build and publish the
scoped package for its own platform.

## Build

From the repository root, build the scoped binary for the current platform with
Bazel:

```sh
npm install --omit=optional --ignore-scripts
./scripts/build-node-prebuild-bazel.sh
```

The script defaults to the host platform; pass an explicit target such as
`win32-x64` for the Bazel + Zig cross-compile path.

This creates:

```text
prebuilds/assets/*.json
prebuilds/assets/*.ocd2
prebuilds/<current-platform>-<current-arch>/opencc.node
```

The release workflow (`release-npm-binaries`) runs the Bazel build script
separately on the runner for each scoped binary target. All Node builds use
Bazel exclusively; the former `binding.gyp`/node-gyp source-build fallback has
been removed.

## Prepare scoped binary packages

After the `prebuilds/` tree exists, generate publishable scoped package
directories:

```sh
npm run prepare:scoped-packages
```

This writes packages under:

```text
dist/scoped-packages/@opencc/opencc-darwin-arm64
dist/scoped-packages/@opencc/opencc-linux-arm64
dist/scoped-packages/@opencc/opencc-linux-x64
dist/scoped-packages/@opencc/opencc-win32-x64
```

Each generated package contains only:

```text
index.js
package.json
README.md
prebuilds/<platform>-<arch>/opencc.node
```

## Verify package contents

Dry-run the main package:

```sh
npm pack --dry-run
```

The main package tarball should contain the JavaScript API and CLI under
`node/`, the TypeScript declarations, and `prebuilds/assets/`, but no C++
sources and not `prebuilds/<platform>/opencc.node`.

Dry-run each scoped package:

```sh
npm pack --dry-run dist/scoped-packages/@opencc/opencc-darwin-arm64
npm pack --dry-run dist/scoped-packages/@opencc/opencc-linux-arm64
npm pack --dry-run dist/scoped-packages/@opencc/opencc-linux-x64
npm pack --dry-run dist/scoped-packages/@opencc/opencc-win32-x64
```

Each scoped tarball should contain one `opencc.node` binary for its own
platform plus a short package-specific README.

## Publish order

Publish the scoped binary packages first. The main package depends on them via
`optionalDependencies`, so they should already exist by the time users install
the main package.

For a prerelease version such as `1.3.2-next1`, use the `next` dist-tag:

```sh
npm publish dist/scoped-packages/@opencc/opencc-darwin-arm64 --access public --tag next
npm publish dist/scoped-packages/@opencc/opencc-linux-arm64 --access public --tag next
npm publish dist/scoped-packages/@opencc/opencc-linux-x64 --access public --tag next
npm publish dist/scoped-packages/@opencc/opencc-win32-x64 --access public --tag next
npm publish --tag next
```

For a stable release, use `latest` or omit `--tag`:

```sh
npm publish dist/scoped-packages/@opencc/opencc-darwin-arm64 --access public
npm publish dist/scoped-packages/@opencc/opencc-linux-arm64 --access public
npm publish dist/scoped-packages/@opencc/opencc-linux-x64 --access public
npm publish dist/scoped-packages/@opencc/opencc-win32-x64 --access public
npm publish
```

`--access public` is required for the first publish of scoped public packages.
It is harmless to keep using it for later publishes.

The final `npm publish` command runs `prepublishOnly`, which verifies that the
scoped binary package versions declared in `optionalDependencies` have already
been published.

## Install-test after publishing

Use a temporary directory and install from npm:

```sh
mkdir /tmp/opencc-install-test
cd /tmp/opencc-install-test
npm init -y
npm install opencc@1.3.2-next1 --tag next
node -e "const OpenCC = require('opencc'); const cc = new OpenCC('s2twp'); console.log(cc.convertSync('鼠标里面的硅二极管坏了'));"
```

The install should pull exactly one matching `@opencc/opencc-<platform>-<arch>`
optional package for the current machine. If no scoped binary package is
available, requiring `opencc` fails with an error explaining where the addon
was looked up.

## CI shape

The CI release flow is:

1. `release-npm-binaries` (workflow_dispatch) builds each scoped binary
   package on its matching platform or runner and publishes them.
2. Pushing a `ver.*` tag creates the draft GitHub release.
3. Publishing the release triggers `release-npm`, which packs and publishes
   the main `opencc` package after verifying the scoped packages exist.

The local flow above remains available for building and publishing a single
platform by hand.
