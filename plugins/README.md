# OpenCC Plugins

`plugins/` contains segmentation plugins that are built and distributed
separately from the OpenCC core library.

The current plugin layout is:

- `plugins/<name>/src/`: plugin implementation and exported entry point
- `plugins/<name>/include/`: plugin-private headers
- `plugins/<name>/data/config/`: plugin-backed JSON configs
- `plugins/<name>/data/<resource-dir>/`: plugin resource files
- `plugins/<name>/tests/`: integration tests for built plugin artifacts

## Design Rules

- The OpenCC core keeps built-in algorithms only. `mmseg` remains built in.
- Any non-built-in `segmentation.type` is resolved through the plugin host.
- A single JSON config must stay platform-neutral. Config files must not embed
  `.so`, `.dylib`, `.dll`, or platform-specific install paths.
- Plugin resources belong to the plugin package, but resource names stay inside
  the normal OpenCC data layout.
- Plugin tests should validate real built artifacts instead of directly testing
  private implementation classes.

## Naming And Installation

Runtime naming follows the segmentation type:

- Linux: `libopencc-<type>.so`
- macOS: `libopencc-<type>.dylib`
- Windows: `opencc-<type>.dll`

Windows loaders also accept the MSYS/MinGW runtime name
`msys-opencc-<type>.dll` when that is the emitted DLL filename.
On Windows, plugins must be built with an ABI-compatible toolchain/runtime as
the host OpenCC binary. Mixing MSVC-built hosts with MinGW-built plugins, or
the reverse, is unsupported.

For the `jieba` plugin, that means:

- Linux: `libopencc-jieba.so`
- macOS: `libopencc-jieba.dylib`
- Windows: `opencc-jieba.dll`

CMake installs plugin binaries into the platform plugin directory and installs
plugin configs/resources into the OpenCC data directory.
Within a single plugin search directory, keep only one DLL for a given
segmentation type. Multiple matching DLL names for the same type are treated as
an error.

## Resource Resolution

Plugin JSON uses resource names rather than platform paths. Example:

```json
{
  "segmentation": {
    "type": "jieba",
    "resources": {
      "dict_path": "jieba_dict/jieba.dict.utf8",
      "model_path": "jieba_dict/hmm_model.utf8",
      "user_dict_path": "jieba_dict/user.dict.utf8"
    }
  }
}
```

The core passes these values to the plugin host. The plugin is responsible for
resolving them at runtime. Relative resource paths are expected to resolve
within the existing OpenCC data layout rather than a plugin-specific ad hoc
directory tree.

## Testing

Each plugin should prefer integration tests that exercise:

- the built `opencc` command or `libopencc`
- the built plugin shared library
- real plugin JSON configs
- real installed or runfiles-based resource files

Current `jieba` targets:

- CMake plugin target: `opencc_jieba`
- CMake integration test: `JiebaPluginIntegrationTest`
- Bazel plugin target: `//plugins/jieba:opencc-jieba`
- Bazel integration test: `//plugins/jieba:jieba_plugin_integration_test`

## Adding A New Plugin

1. Create `plugins/<name>/src`, `include`, `data`, and `tests`.
2. Export `opencc_get_segmentation_plugin_v1()`.
3. Name the output binary using the `opencc-<type>` convention.
4. Keep JSON configs platform-neutral and resource-oriented.
  5. Add both CMake and Bazel build rules.
  6. Add an integration test that loads the built plugin through the real host.

  ## Packaging for Distro Maintainers

  To align with downstream Linux distribution packaging standards (e.g., Debian `apt`, Arch `pacman`), OpenCC plugins strongly support **decoupled compilation**. This allows maintainers to build and distribute the core `opencc` system separately from heavy third-party plugins like `opencc-jieba`.

  ### 1. Build and Install Core OpenCC
  Compile the main directory normally but ensure plugins are disabled.
  ```bash
  mkdir build_core && cd build_core
  cmake .. -DBUILD_OPENCC_JIEBA_PLUGIN=OFF -DCMAKE_INSTALL_PREFIX=/usr
  make && make install
  ```

  ### 2. Build the Plugin Standalone
  Plugins can automatically detect if they are being built standalone. CD directly into the plugin directory and point `OpenCC_DIR` to the CMake registry established in step 1.
  ```bash
  cd plugins/jieba
  mkdir build_plugin && cd build_plugin
  cmake .. -DOpenCC_DIR=/usr/lib/cmake/opencc -DCMAKE_INSTALL_PREFIX=/usr
  make && make install
  ```
  *(Note: Standalone default installation paths like `DIR_PLUGIN` will natively align with the core OpenCC configuration, e.g., `/usr/lib/opencc/plugins`)*
