# Publishing opencc-jieba npm packages

This document describes the current experimental release flow for the
`opencc-jieba` npm package family.

The split-package layout is intentionally being tried on `opencc-jieba` first:

- `opencc-jieba` contains JavaScript, Jieba configs, and Jieba dictionary data.
- `@opencc/opencc-jieba-<platform>-<arch>` contains exactly one native plugin
  library for one platform.

If this install and publishing model works well, the same approach can later be
applied to the main `opencc` npm package.

## Package names

The main package declares the scoped packages as optional dependencies:

```text
opencc-jieba
@opencc/opencc-jieba-darwin-arm64
@opencc/opencc-jieba-linux-arm64
@opencc/opencc-jieba-linux-x64
@opencc/opencc-jieba-win32-x64
```

The versions must stay in sync. For example, when publishing
`opencc-jieba@1.3.2-next1`, every scoped binary package should also be published
as `1.3.2-next1`.

## Prerequisites

- npm account access to publish `opencc-jieba`.
- npm organization access to publish public packages under `@opencc`.
- Bazel configured for the platform builds used by `build.js`.
- For the current local all-in-one flow, an ARM64 macOS machine with the remote
  Linux build configs available.

At the moment, maintainers are assumed to run this release process from an
ARM64 macOS development machine. In the future, the per-platform build and
publish steps should move to GitHub Actions so the release process no longer
depends on the maintainer's local platform.

## Build

From the repository root:

```sh
cd plugins/jieba/node
npm run build:all
```

This creates:

```text
data/
prebuilds/darwin-arm64/libopencc-jieba.dylib
prebuilds/linux-arm64/libopencc-jieba.so
prebuilds/linux-x64/libopencc-jieba.so
prebuilds/win32-x64/opencc-jieba.dll
```

For one platform only:

```sh
npm run build
npm run build linux-x64
npm run build linux-arm64
npm run build win32-x64
```

`npm run build` builds the host platform, so on an Apple Silicon Mac it produces
`darwin-arm64`.

## Prepare scoped binary packages

After the `prebuilds/` tree exists, generate publishable scoped package
directories:

```sh
npm run prepare:scoped-packages
```

This writes packages under:

```text
dist/scoped-packages/@opencc/opencc-jieba-darwin-arm64
dist/scoped-packages/@opencc/opencc-jieba-linux-arm64
dist/scoped-packages/@opencc/opencc-jieba-linux-x64
dist/scoped-packages/@opencc/opencc-jieba-win32-x64
```

Each generated package contains only:

```text
index.js
package.json
README.md
prebuilds/<platform>-<arch>/<native-library>
```

## Verify package contents

Dry-run the main package without running `prepack` again:

```sh
npm pack --dry-run --ignore-scripts
```

The main package tarball should contain `index.js`, `README.md`, `package.json`,
and `data/`, but not `prebuilds/`.

Dry-run each scoped package:

```sh
npm pack --dry-run dist/scoped-packages/@opencc/opencc-jieba-darwin-arm64
npm pack --dry-run dist/scoped-packages/@opencc/opencc-jieba-linux-arm64
npm pack --dry-run dist/scoped-packages/@opencc/opencc-jieba-linux-x64
npm pack --dry-run dist/scoped-packages/@opencc/opencc-jieba-win32-x64
```

Each scoped tarball should contain one native library for its own platform plus
a short package-specific README.

## Publish order

Publish the scoped binary packages first. The main package depends on them via
`optionalDependencies`, so they should already exist by the time users install
the main package.

For a prerelease version such as `1.3.2-next1`, use the `next` dist-tag:

```sh
npm publish dist/scoped-packages/@opencc/opencc-jieba-darwin-arm64 --access public --tag next
npm publish dist/scoped-packages/@opencc/opencc-jieba-linux-arm64 --access public --tag next
npm publish dist/scoped-packages/@opencc/opencc-jieba-linux-x64 --access public --tag next
npm publish dist/scoped-packages/@opencc/opencc-jieba-win32-x64 --access public --tag next
npm publish --tag next
```

For a stable release, use `latest` or omit `--tag`:

```sh
npm publish dist/scoped-packages/@opencc/opencc-jieba-darwin-arm64 --access public
npm publish dist/scoped-packages/@opencc/opencc-jieba-linux-arm64 --access public
npm publish dist/scoped-packages/@opencc/opencc-jieba-linux-x64 --access public
npm publish dist/scoped-packages/@opencc/opencc-jieba-win32-x64 --access public
npm publish
```

`--access public` is required for the first publish of scoped public packages.
It is harmless to keep using it for later publishes.

## Install-test after publishing

Use a temporary directory and install both packages from npm:

```sh
mkdir /tmp/opencc-jieba-install-test
cd /tmp/opencc-jieba-install-test
npm init -y
npm install opencc@1.3.2-next1 opencc-jieba@1.3.2-next1 --tag next
node -e "const OpenCC = require('opencc'); const cc = new OpenCC('s2twp_jieba'); cc.convert('鼠标里面的硅二极管坏了', (err, text) => { if (err) throw err; console.log(text); });"
```

The install should pull exactly one matching
`@opencc/opencc-jieba-<platform>-<arch>` optional package for the current
machine.

## Future CI shape

The intended CI release flow is:

1. Build each scoped binary package on its matching platform or runner.
2. Upload scoped package artifacts.
3. Publish all scoped binary packages with the chosen version and dist-tag.
4. Publish the main `opencc-jieba` package after the scoped packages exist.

Until that CI flow exists, the local flow above is the source of truth.
