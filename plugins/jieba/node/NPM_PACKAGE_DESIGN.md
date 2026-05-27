# opencc-jieba Node.js Architecture & Engineering Design

> This document details the architectural improvements and engineering updates for the `opencc-jieba` Node.js package.

## 1. Background and Objectives

Integrating the Jieba segmentation plugin into Node.js previously faced two major pain points:
1. **Environment Variable Dependency**: The C++ core library relied on environment variables (like `OPENCC_SEGMENTATION_PLUGIN_PATH` and `OPENCC_DATA_DIR`) to locate dynamic libraries and dictionary files. This approach is prone to global pollution and is unsuitable for an independent NPM module.
2. **Binary Artifact Management**: Committing large cross-platform build artifacts (`.dylib`, `.so`, `.dll`) and dictionary data directly into the Git repository causes uncontrollable repository bloat and severe merge conflicts during CI workflows and collaborative development.

To resolve these issues, we refactored `opencc-jieba` into a **fully self-contained, deterministic, and artifact-free** modern NPM package.

---

## 2. Core Architectural Changes

### 2.1 Deterministic Plugin Loading via Absolute Paths

**[C++ Core Updates: `src/PluginSegmentation.cpp`]**
To eliminate reliance on environment variables, we introduced a precise loading entry point in the C++ `PluginManager`: `LoadByPath(const std::string& path, ...)`.
Simultaneously, when parsing segmentation configurations, the core library now checks for a special parameter named `__plugin_library` within the config pairs:
```cpp
// If __plugin_library is specified, bypass the traditional directory search strategy
std::shared_ptr<PluginLibrary> plugin =
    pluginLibraryPath.empty()
        ? GetPluginManager().LoadByType(type)
        : GetPluginManager().LoadByPath(pluginLibraryPath, type);
```

**[Node.js Integration]**
On the Node.js side, `plugins/jieba/node/index.js` now exports the exact absolute paths to the platform-specific dynamic library (`pluginLibrary`) and the data directory (`dataDir`).
When the upper-level Node.js wrapper (e.g., the `opencc` npm package) parses user-specified JSON configurations, it dynamically injects these **absolute paths** into the `__plugin_library` field. This guarantees 100% stable plugin discovery, completely unaffected by environment variables.

### 2.2 Separation of Artifacts and Source Code

**[Cleaning up Git Tracking: `.gitignore`]**
By adding `prebuilds/` and `data/` to `plugins/jieba/node/.gitignore`, we completely stopped tracking external dictionary copies and compiled dynamic libraries in the code repository.

**[Automated Directory Tree Restoration: `build.js`]**
To ensure a seamless NPM publishing experience, we introduced `plugins/jieba/node/build.js`. Before packaging, this script automates the following pipeline:
1. **Triggering the Build System**: Executes `bazel build //plugins/jieba:opencc-jieba //plugins/jieba:jieba_merged_dict` to compile the native addon and convert dictionaries.
2. **Collecting Artifacts**:
   - Extracts the generated dynamic libraries (`.dylib` / `.so` / `.dll`) into `prebuilds/<platform>-<arch>/`.
   - Copies the upstream raw dictionaries and the compiled `.ocd2` binary dictionary into `data/jieba_dict/`.
   - Copies segmentation-specific JSON configurations into `data/`.

**[Automated NPM Hooks: `package.json`]**
We configured `"prepack": "npm run build"` in the `scripts` section of `package.json`. This ensures that whether running `npm pack` locally or `npm publish` in CI, NPM will automatically invoke `build.js` to populate the latest artifact directories. End users running `npm install` simply download the prebuilt package without needing to compile from source or install Bazel.

### 2.3 Highly Optimized Cross-Platform Compilation

Building upon this engineering foundation, we optimized the package size and distribution efficiency:
1. **Symbol Stripping**: Modified `plugins/jieba/BUILD.bazel` to explicitly add `-Wl,-S` (macOS) and `-Wl,-s` (Linux) linkopts. This ensures that C++ dynamic libraries in Release mode are stripped of redundant debugging symbols.
2. **Remote Cross-Compilation**: The `build.js` script now accepts CLI arguments to trigger remote compilation (e.g., `--config=linux-arm64-remote`). Coupled with the `--remote_download_toplevel` Bazel flag, developers can seamlessly trigger cross-platform builds via a remote execution cluster and accurately download the artifacts directly into the local packaging directory tree.

---

## 3. Conclusion

Through these refactoring efforts, the development and publishing workflow for `opencc-jieba` is now highly standardized. For contributors, the repository remains clean, and updating code or dictionaries requires only a single command to rebuild and repackage. For end users, the plugin provides a true "plug-and-play" experience, eliminating all elusive bugs caused by environment variable misconfigurations.
