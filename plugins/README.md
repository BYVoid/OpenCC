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

For the `jieba` plugin, that means:

- Linux: `libopencc-jieba.so`
- macOS: `libopencc-jieba.dylib`
- Windows: `opencc-jieba.dll`

CMake installs plugin binaries into the platform plugin directory and installs
plugin configs/resources into the OpenCC data directory.

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
