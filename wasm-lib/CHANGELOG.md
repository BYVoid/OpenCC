# Changelog

All notable changes to opencc-wasm will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.3.0] - 2026-01-03

### Changed

**🚨 BREAKING: New Distribution Layout**

The `.wasm` files have been moved to be co-located with their corresponding glue code files. This fixes loading issues in various environments and enables proper CDN usage.

**Before (0.2.x):**
```
dist/
  esm/
    index.js
    opencc-wasm.js
  cjs/
    index.cjs
    opencc-wasm.cjs
  opencc-wasm.wasm          ← Was only here
  opencc-wasm.esm.wasm
```

**After (0.3.0):**
```
dist/
  esm/
    index.js
    opencc-wasm.js
    opencc-wasm.wasm        ← Now here (same directory as glue code)
  cjs/
    index.cjs
    opencc-wasm.cjs
    opencc-wasm.wasm        ← Now here (same directory as glue code)
  opencc-wasm.wasm          ← Kept for legacy compatibility
```

**Migration Guide:**

If you were importing the WASM module directly (not the high-level API), no changes are needed. The new layout is automatically handled by the build system.

### Added

- **CDN Support**: Package can now be used directly from CDN (jsDelivr, unpkg, etc.)
  ```javascript
  import OpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.3.0/dist/esm/index.js";
  ```

- **Comprehensive Test Suite**: Added CDN usage tests
  - `npm test` now runs both core tests (56 cases) and CDN usage tests
  - `npm run test:core` - Run core functionality tests only
  - `npm run test:cdn` - Run CDN usage tests only
  - Added `test/cdn-simple.mjs` - High-level API test
  - Added `test/cdn-usage.mjs` - Low-level WASM API test
  - Added `test/cdn-test.html` - Browser environment test page

- **Documentation**: New comprehensive guides
  - `test/CDN_USAGE.md` - Complete CDN usage guide with examples
  - `test/README.md` - Test suite overview
  - `test/TESTING.md` - Detailed testing guide

### Fixed

- **WASM Loading**: Fixed `.wasm` file not found errors in various bundlers and environments
- **Emscripten Glue Code**: Updated `locateFile` paths to correctly resolve `.wasm` files from the same directory as glue code
- **Build System**: Fixed regex replacement to handle both escaped and literal dots in WASM filename references

### Internal

- Updated `build-api.js` to copy `.wasm` files to both `esm/` and `cjs/` directories
- Updated `index.js` locateFile to use relative paths from glue code location
- Removed obsolete `opencc-wasm.esm.wasm` naming

---

## [0.2.1] - 2024-12-xx

### Fixed

- Copy both wasm binaries for compatibility

---

## [0.2.0] - 2024-12-xx

### Changed

- Initial WASM distribution structure

---

## [Unreleased]

*No unreleased changes*

---

[0.3.0]: https://github.com/frankslin/OpenCC/compare/v0.2.1...v0.3.0
[0.2.1]: https://github.com/frankslin/OpenCC/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/frankslin/OpenCC/releases/tag/v0.2.0
