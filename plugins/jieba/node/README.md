# opencc-jieba

Jieba segmentation plugin for OpenCC (Node.js).

This package provides Jieba dictionary data (`.ocd2`, `.utf8`) and loads the matching pre-compiled native dynamic library (`.dylib`, `.so`, `.dll`) from an optional platform package such as `@opencc/opencc-jieba-darwin-arm64` or `@opencc/opencc-jieba-linux-x64`.

By leveraging this standalone NPM package, users do not need to manually install C++ toolchains or configure complex environment variables (such as `OPENCC_SEGMENTATION_PLUGIN_PATH` or `OPENCC_DATA_DIR`). The prebuilt libraries are deterministic and fully self-contained.

## Installation

```sh
npm install opencc-jieba
```

> **Note**: This package is intended to be used alongside the `opencc` npm package (`>= 1.3.2`).
> npm installs the matching optional `@opencc/opencc-jieba-<platform>-<arch>` package automatically on supported platforms.

Maintainers should follow [PUBLISHING.md](PUBLISHING.md) for the split-package
build and release flow.

## Usage

This package acts as an underlying plugin provider. It exports absolute paths to the cross-platform native binaries and dictionary files, which can then be dynamically injected into the OpenCC configuration.

```javascript
const openccJieba = require('opencc-jieba');

// Absolute path to the directory containing the dynamic library
console.log(openccJieba.pluginDir);
// e.g. ".../node_modules/@opencc/opencc-jieba-darwin-arm64/prebuilds/darwin-arm64"

// Absolute path to the specific dynamic library file
console.log(openccJieba.pluginLibrary);
// e.g. ".../node_modules/@opencc/opencc-jieba-darwin-arm64/prebuilds/darwin-arm64/libopencc-jieba.dylib"

// Absolute path to the data directory containing config JSONs and dictionaries
console.log(openccJieba.dataDir);
// e.g. ".../node_modules/opencc-jieba/data"
```

### Integration with OpenCC

When initializing OpenCC in Node.js, you can supply these paths to override the default search directories, ensuring that the plugin is reliably discovered regardless of the runtime environment.

## License

Apache License 2.0
