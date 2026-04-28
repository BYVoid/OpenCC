# opencc-jieba

Jieba segmentation plugin for OpenCC (Node.js).

This package provides pre-compiled native dynamic libraries (`.dylib`, `.so`, `.dll`) and Jieba dictionary data (`.ocd2`, `.utf8`) to allow [OpenCC](https://github.com/BYVoid/OpenCC) to use the Jieba algorithm for Chinese text segmentation.

By leveraging this standalone NPM package, users do not need to manually install C++ toolchains or configure complex environment variables (such as `OPENCC_SEGMENTATION_PLUGIN_PATH` or `OPENCC_DATA_DIR`). The prebuilt libraries are deterministic and fully self-contained.

## Installation

```sh
npm install opencc-jieba
```

> **Note**: This package is intended to be used alongside the `opencc` npm package (`>= 1.3.1`).

## Usage

This package acts as an underlying plugin provider. It exports absolute paths to the cross-platform native binaries and dictionary files, which can then be dynamically injected into the OpenCC configuration.

```javascript
const openccJieba = require('opencc-jieba');

// Absolute path to the directory containing the dynamic library
console.log(openccJieba.pluginDir);
// e.g. ".../node_modules/opencc-jieba/prebuilds/darwin-arm64"

// Absolute path to the specific dynamic library file
console.log(openccJieba.pluginLibrary);
// e.g. ".../node_modules/opencc-jieba/prebuilds/darwin-arm64/libopencc-jieba.dylib"

// Absolute path to the data directory containing config JSONs and dictionaries
console.log(openccJieba.dataDir);
// e.g. ".../node_modules/opencc-jieba/data"
```

### Integration with OpenCC

When initializing OpenCC in Node.js, you can supply these paths to override the default search directories, ensuring that the plugin is reliably discovered regardless of the runtime environment.

## License

Apache License 2.0
