# OpenCC WASM CDN 使用指南

## 新布局结构

```
dist/
  esm/
    index.js              ← 高级 API（推荐）
    opencc-wasm.js        ← WASM glue code
    opencc-wasm.wasm      ← WASM 二进制文件（与 .js 同目录）
  cjs/
    index.cjs             ← CommonJS 高级 API
    opencc-wasm.cjs       ← WASM glue code (CJS)
    opencc-wasm.wasm      ← WASM 二进制文件（与 .cjs 同目录）
  opencc-wasm.wasm        ← Legacy 副本
  data/
    config/               ← JSON 配置文件
    dict/                 ← .ocd2 字典文件
```

## 方式 1: 使用高级 API（推荐）

这是最简单的使用方式：

```javascript
import OpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/index.js";

// 创建转换器
const converter = OpenCC.Converter({ from: "cn", to: "t" });

// 转换文本
const result = await converter("简体中文");
console.log(result);  // 输出: 簡體中文
```

### 支持的转换类型

```javascript
// 简体 → 繁体
OpenCC.Converter({ from: "cn", to: "t" })

// 简体 → 台湾繁体
OpenCC.Converter({ from: "cn", to: "tw" })

// 简体 → 香港繁体
OpenCC.Converter({ from: "cn", to: "hk" })

// 繁体 → 简体
OpenCC.Converter({ from: "t", to: "cn" })

// 台湾繁体 → 简体
OpenCC.Converter({ from: "tw", to: "cn" })

// 香港繁体 → 简体
OpenCC.Converter({ from: "hk", to: "cn" })
```

### 自定义词典

```javascript
// 组合官方转换和自定义词典
const converter = OpenCC.ConverterFactory("cn", "tw", [
  [["服务器", "伺服器"], ["文件", "檔案"]],
  "網路 网络"
]);

const result = await converter("服务器上的文件通过网络传输");
// 输出: 伺服器上的檔案通過網路傳輸
```

## 方式 2: 直接使用 WASM 模块（高级）

如果你需要完全控制，可以直接导入 WASM glue code：

```javascript
import initOpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/opencc-wasm.js";

// 初始化 WASM 模块
const wasmModule = await initOpenCC();

// 包装 C API
const api = {
  create: wasmModule.cwrap("opencc_create", "number", ["string"]),
  convert: wasmModule.cwrap("opencc_convert", "string", ["number", "string"]),
  destroy: wasmModule.cwrap("opencc_destroy", null, ["number"]),
};

// 设置虚拟文件系统并加载配置/字典
wasmModule.FS.mkdirTree("/data/config");
wasmModule.FS.mkdirTree("/data/dict");

// ... 加载配置和字典文件到 FS ...

// 创建转换器
const handle = api.create("/data/config/s2t.json");

// 转换文本
const result = api.convert(handle, "简体中文");

// 清理
api.destroy(handle);
```

## 浏览器中使用

### 基础示例

```html
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>OpenCC WASM 示例</title>
</head>
<body>
  <textarea id="input">开源中国转换工具</textarea>
  <button id="convert">转换</button>
  <div id="output"></div>

  <script type="module">
    import OpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/index.js";

    const converter = OpenCC.Converter({ from: "cn", to: "t" });

    document.getElementById('convert').onclick = async () => {
      const text = document.getElementById('input').value;
      const result = await converter(text);
      document.getElementById('output').textContent = result;
    };
  </script>
</body>
</html>
```

### 性能优化建议

1. **缓存转换器实例**：只创建一次，重复使用
2. **Web Worker**：对于大文本转换，考虑在 worker 中执行
3. **压缩传输**：配置 CDN 启用 gzip/brotli
4. **Service Worker**：缓存 .wasm 和字典文件

## Node.js 中使用

### ESM (推荐)

```javascript
import OpenCC from "opencc-wasm";

const converter = OpenCC.Converter({ from: "cn", to: "t" });
const result = await converter("简体中文");
```

### CommonJS

```javascript
const OpenCC = require("opencc-wasm");

const converter = OpenCC.Converter({ from: "cn", to: "t" });
converter("简体中文").then(console.log);
```

## 测试

运行测试脚本验证安装：

```bash
# 高级 API 测试
node test/cdn-simple.mjs

# 低级 WASM API 测试
node test/cdn-usage.mjs

# 浏览器测试（需要启动 HTTP 服务器）
python3 -m http.server 8888
# 访问 http://localhost:8888/test/cdn-test.html
```

## 故障排查

### 问题：找不到 .wasm 文件

**症状**：`ENOENT: no such file or directory, open '.../opencc-wasm.wasm'`

**解决**：确保 `.wasm` 文件与对应的 `.js/.cjs` glue code 在同一目录下

### 问题：CORS 错误

**症状**：`CORS policy: No 'Access-Control-Allow-Origin' header`

**解决**：
- 使用支持 CORS 的 CDN（如 jsdelivr、unpkg）
- 或在本地服务器配置 CORS 头部

### 问题：bundler 找不到模块

**症状**：Webpack/Vite 报错找不到 .wasm 文件

**解决**：
- Webpack: 添加 `experiments: { asyncWebAssembly: true }`
- Vite: 添加 `assetsInclude: ['**/*.wasm']`

## 版本说明

- **0.2.1+**: 新布局，`.wasm` 文件与 glue code 同目录
- **0.2.0**: 旧布局，`.wasm` 文件在根目录

## 许可证

Apache-2.0

## 参考资料

- [官方 OpenCC 项目](https://github.com/BYVoid/OpenCC)
- [问题反馈](https://github.com/frankslin/OpenCC/issues)
