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

## Build

The project uses a two-stage build process with semantic separation:

### Stage 1: Build WASM (intermediate artifacts)

```bash
./build.sh
```

Compiles OpenCC + marisa-trie to WASM and generates intermediate build artifacts in `build/`:
- `build/opencc-wasm.esm.js` - ESM WASM glue (for tests/development)
- `build/opencc-wasm.cjs` - CJS WASM glue (for tests/development)
- `build/opencc-wasm.wasm` - WASM binary

**Semantic: `build/` = internal intermediate artifacts, not for publishing**

### Stage 2: Build API wrappers (publishable dist)

```bash
node scripts/build-api.js
```

Generates publishable distribution in `dist/`:
- Copies WASM artifacts from `build/` to `dist/esm/` and `dist/cjs/`
- Transforms source `index.js` to `dist/esm/index.js` with production paths
- Generates `dist/cjs/index.cjs` with CJS-compatible wrapper
- Copies data files to `dist/data/`

**Semantic: `dist/` = publishable artifacts for npm**

### Complete build

```bash
npm run build
```

Runs both stages automatically.

## Testing
```bash
npm test
```

Tests import from source `index.js`, which references `build/` artifacts.
This ensures tests validate the actual build output, not stale dist files.

Runs the upstream OpenCC testcases (converted to JSON) against the WASM build.

## Project Structure

```
wasm-lib/
├── build/              ← Intermediate WASM artifacts (gitignored, for tests)
│   ├── opencc-wasm.esm.js
│   ├── opencc-wasm.cjs
│   └── opencc-wasm.wasm
├── dist/               ← Publishable distribution (committed to git)
│   ├── esm/
│   │   ├── index.js
│   │   └── opencc-wasm.js
│   ├── cjs/
│   │   ├── index.cjs
│   │   └── opencc-wasm.cjs
│   ├── opencc-wasm.wasm
│   └── data/           ← OpenCC config + dict files
├── index.js            ← Source API (references build/ for tests)
├── index.d.ts          ← TypeScript definitions
└── scripts/
    └── build-api.js    ← Transforms build/ → dist/
```

**Invariants:**
- Tests import source (`index.js`) → loads from `build/`
- Published package exports `dist/` only
- `build/` = internal, `dist/` = publishable

## Notes
- Internally uses persistent OpenCC handles (`opencc_create/convert/destroy`) to avoid reloading configs.
- Dictionaries are written under `/data/dict/` in the virtual FS; configs under `/data/config/`. Paths inside configs are rewritten automatically.
- Memory grows on demand (`ALLOW_MEMORY_GROWTH=1`); no native dependencies needed.
- Performance note: opencc-wasm focuses on fidelity and compatibility (uses official configs and `.ocd2`, matches Node OpenCC output 1:1). Raw throughput can be slower than pure JS implementations like `opencc-js`, but the WASM version guarantees full OpenCC behavior and config coverage.

## 0.2.0 changes
- Conversion rules and bundled dictionaries are rebuilt from OpenCC commit [`36c7cbbc`](https://github.com/frankslin/OpenCC/commit/36c7cbbc9702d2a46a89ea7a55ff8ba5656455df). This aligns the WASM build with the upstream configs in that revision (including updated `.ocd2` data).
- Output layout now mirrors the new `dist/` structure: ESM glue under `dist/esm/`, CJS glue under `dist/cjs/`, shared `opencc-wasm.wasm` at `dist/opencc-wasm.wasm`, and configs/dicts in `dist/data/`. Adjust your bundler/static hosting paths accordingly.
- Tests are rewritten to use `node:test` with data-driven cases (`test/testcases.json`) instead of ad-hoc assertions, keeping coverage aligned with upstream OpenCC fixtures.
