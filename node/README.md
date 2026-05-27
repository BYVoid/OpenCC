# OpenCC for Node.js

OpenCC provides high quality conversion between Simplified Chinese,
Traditional Chinese, and regional variants. This package contains the official
Node.js bindings for the OpenCC native library.

## Installation

```bash
npm install opencc
```

The package requires Node.js `>=20.17`.

OpenCC ships native bindings. When a matching prebuilt binary is available,
installation uses it automatically. Otherwise npm builds the native addon from
source, which requires a working C++ toolchain and Python supported by
`node-gyp`.

## Basic Usage

ES modules:

```js
import OpenCC from 'opencc';

const converter = new OpenCC('s2t.json');
console.log(converter.convertSync('汉字'));
```

Promise API:

```js
import OpenCC from 'opencc';

const converter = new OpenCC('s2tw.json');
console.log(await converter.convertPromise('鼠标和软件'));
```

Named import:

```js
import { OpenCC } from 'opencc';

const converter = new OpenCC('t2s.json');
console.log(converter.convertSync('漢字'));
```

CommonJS is still supported for older projects:

```js
const { OpenCC } = require('opencc');

const converter = new OpenCC('s2t.json');
console.log(converter.convertSync('汉字'));
```

Callback API:

```js
import OpenCC from 'opencc';

const converter = new OpenCC('t2s.json');

converter.convert('漢字', (err, text) => {
  if (err) throw err;
  console.log(text);
});
```

TypeScript type declarations are included.

## API

### `new OpenCC(config?)`

Creates a converter. If `config` is omitted, OpenCC uses `s2t.json`.

`config` can be one of the built-in configuration file names, such as
`s2t.json`, or an absolute path to a custom OpenCC configuration file.

### `converter.convertSync(input)`

Converts text synchronously and returns the converted string.

### `converter.convert(input, callback)`

Converts text asynchronously.

The callback receives `(err, convertedText)`.

### `converter.convertPromise(input)`

Converts text asynchronously and returns a `Promise<string>`.

### `OpenCC.version`

The version string of the bundled OpenCC native library.

### `OpenCC.generateDict(inputFileName, outputFileName, formatFrom, formatTo)`

Generates a dictionary file from another dictionary format.

For example, convert a text dictionary to OpenCC's binary `ocd2` format:

```js
import { OpenCC } from 'opencc';

OpenCC.generateDict('my-dict.txt', 'my-dict.ocd2', 'text', 'ocd2');
```

## Command Line

Install OpenCC globally to use the npm command line tool:

```bash
npm install -g opencc
```

The package installs an `opencc` command:

```bash
opencc -c s2t.json -i input.txt -o output.txt
```

It can also read from stdin and write to stdout:

```bash
echo '汉字' | opencc -c s2t.json
```

Options:

```text
-c, --config <file>  Configuration file. Defaults to s2t.json.
-i, --input <file>   Read original text from <file>. Defaults to stdin.
-o, --output <file>  Write converted text to <file>. Defaults to stdout.
-v, --version        Print OpenCC version.
-h, --help           Print help.
```

The npm CLI supports conversion only. Use the native OpenCC command line tool
for inspection, segmentation output, custom resource search paths, or other
advanced CLI features.

## Built-in Configurations

| Configuration | Conversion |
| --- | --- |
| `s2t.json` | Simplified Chinese to Traditional Chinese (OpenCC Standard) / 簡體到 OpenCC 標準繁體 |
| `t2s.json` | Traditional Chinese (OpenCC Standard) to Simplified Chinese / OpenCC 標準繁體到簡體 |
| `s2tw.json` | Simplified Chinese to Traditional Chinese (Taiwan Standard) / 簡體到台灣正體 |
| `tw2s.json` | Traditional Chinese (Taiwan Standard) to Simplified Chinese / 台灣正體到簡體 |
| `s2hk.json` | Simplified Chinese to Traditional Chinese (Hong Kong variant) / 簡體到香港繁體 |
| `hk2s.json` | Traditional Chinese (Hong Kong variant) to Simplified Chinese / 香港繁體到簡體 |
| `s2twp.json` | Simplified Chinese to Traditional Chinese (Taiwan Standard) with Taiwanese idiom / 簡體到台灣正體，並轉換為台灣常用詞彙 |
| `tw2sp.json` | Traditional Chinese (Taiwan Standard) to Simplified Chinese with Mainland Chinese idiom / 台灣正體到簡體，並轉換為中國大陸常用詞彙 |
| `t2tw.json` | Traditional Chinese (OpenCC Standard) to Traditional Chinese (Taiwan Standard) / OpenCC 標準繁體到台灣正體 |
| `tw2t.json` | Traditional Chinese (Taiwan Standard) to Traditional Chinese (OpenCC Standard) / 台灣正體到 OpenCC 標準繁體 |
| `t2hk.json` | Traditional Chinese (OpenCC Standard) to Traditional Chinese (Hong Kong variant) / OpenCC 標準繁體到香港繁體 |
| `hk2t.json` | Traditional Chinese (Hong Kong variant) to Traditional Chinese (OpenCC Standard) / 香港繁體到 OpenCC 標準繁體 |
| `t2jp.json` | Traditional Chinese Characters (Kyūjitai) to New Japanese Kanji (Shinjitai) / OpenCC 標準繁體（日文舊字體）到日文新字體 |
| `jp2t.json` | New Japanese Kanji (Shinjitai) to Traditional Chinese Characters (Kyūjitai) / 日文新字體到 OpenCC 標準繁體（日文舊字體） |

## Custom Configurations

Pass a path to a custom OpenCC configuration JSON file:

```js
import OpenCC from 'opencc';

const converter = new OpenCC('/absolute/path/to/custom.json');
console.log(converter.convertSync('汉字'));
```

Relative config paths are resolved from the current working directory by the
CLI. In the JavaScript API, relative config names are resolved from the package
assets directory, so custom configs should be passed as absolute paths.

Custom dictionaries can be generated with `OpenCC.generateDict()` or with the
native `opencc_dict` tool from the full OpenCC distribution.

## Optional Jieba Configurations

Some OpenCC configurations use jieba segmentation. Install the optional
`opencc-jieba` package to make jieba-backed configs available:

```bash
npm install opencc opencc-jieba
```

When `opencc-jieba` is installed, the JavaScript API and npm `opencc` CLI can
load its configs automatically:

```js
import OpenCC from 'opencc';

const converter = new OpenCC('s2twp_jieba');
console.log(converter.convertSync('软件鼠标'));
```

The npm CLI also accepts the same config names for text conversion:

```bash
opencc -c s2twp_jieba
```

Use the native OpenCC CLI for diagnostic modes such as `--inspect` and
`--segmentation`.

## Related npm Packages

The [`opencc`](https://www.npmjs.com/package/opencc) npm package is the
Node.js native binding for the official OpenCC C++ project. It is intended for
Node.js, depends on native or prebuilt binaries, and follows the official
OpenCC engine. It can use extended segmentation algorithms such as Jieba when
supported by the official OpenCC configuration and runtime.

The [`opencc-js`](https://www.npmjs.com/package/opencc-js) npm package is a
pure JavaScript implementation for browsers and Node.js. It bundles dictionary
data generated from `opencc-data`, so it does not require native binaries and
does not fetch dictionary text files at runtime.

`opencc-js` has aligned its conversion flow with the official OpenCC
implementation, including phrase segmentation for built-in converters, and is
tested against upstream OpenCC test cases and golden outputs. It still should
not be treated as guaranteed to produce 100% identical results for every
possible input.

`opencc-js` currently supports the built-in OpenCC mmseg-style segmentation
used by its bundled converters, but it does not support extended segmentation
algorithms such as Jieba.

The [`opencc-wasm`](https://www.npmjs.com/package/opencc-wasm) npm package is
another browser-capable implementation. It uses WebAssembly, keeps its
configuration and conversion logic aligned with the official `opencc` package,
and can support Jieba segmentation through the official OpenCC runtime.

## Development

From the repository root:

```bash
npm install
npm test
```

To build prebuilt native addons for publishing, see
[`node/PUBLISHING.md`](./PUBLISHING.md).
