# opencc-wasm

This package provides a WebAssembly backend for OpenCC, fully compatible with the `opencc-js` public API. It bundles the OpenCC C++ core (plus marisa) compiled via Emscripten, plus the official OpenCC configs and prebuilt `.ocd2` dictionaries (placed under `dist/data/` at build time).
License: Apache-2.0 (see LICENSE).

## Features
- Same API surface as `opencc-js`: `OpenCC.Converter`, `CustomConverter`, `ConverterFactory`, and locale presets.
- No native bindings required; runs in Node.js and modern browsers (ESM), with a CommonJS build for legacy `require`.
- On-demand loading of configs and dictionaries from the package’s `data/` directory into the Emscripten FS; each config/dict is cached after first use.

## Installation
```bash
npm install opencc-wasm
```

## Usage
```js
import OpenCC from "opencc-wasm";

// Convert Traditional Chinese (HK) to Simplified (CN)
const converter = OpenCC.Converter({ from: "hk", to: "cn" });
console.log(await converter("漢語")); // => 汉语

// Custom dictionary
const custom = OpenCC.CustomConverter([
  ["“", "「"],
  ["”", "」"],
  ["‘", "『"],
  ["’", "』"],
]);
console.log(custom("悟空道：“师父又来了。怎么叫做‘水中捞月’？”"));
// => 悟空道：「師父又來了。怎麼叫做『水中撈月』？」
```

### Node (CommonJS)
```js
const OpenCC = require("opencc-wasm").default;
```

## Files and entry points
- ESM: `dist/opencc-wasm.js`
- CJS: `dist/opencc-wasm.cjs`
- Data: `dist/data/config/*.json`, `dist/data/dict/*.ocd2` (copied during `npm run build`)

The package `exports` map is set so bundlers and Node can pick the right build automatically.

## Testing
```bash
cd wasm-lib
npm test
```
Runs the upstream OpenCC testcases (converted to JSON) against the WASM build.

## Notes
- Internally uses persistent OpenCC handles (`opencc_create/convert/destroy`) to avoid reloading configs.
- Dictionaries are written under `/data/dict/` in the virtual FS; configs under `/data/config/`. Paths inside configs are rewritten automatically.
- Memory grows on demand (`ALLOW_MEMORY_GROWTH=1`); no native dependencies needed.
- Performance note: opencc-wasm focuses on fidelity and compatibility (uses official configs and `.ocd2`, matches Node OpenCC output 1:1). Raw throughput can be slower than pure JS implementations like `opencc-js`, but the WASM version guarantees full OpenCC behavior and config coverage.
