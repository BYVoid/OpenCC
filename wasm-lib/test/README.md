# OpenCC WASM 测试套件

本目录包含 opencc-wasm 的各种测试，验证不同使用场景。

## 测试文件说明

### 核心功能测试

#### `opencc.test.js`
主要的单元测试套件，测试所有转换配置。
```bash
npm test
```

### CDN 使用模式测试

#### `cdn-simple.mjs` ⭐ 推荐
测试**高级 API**（最常用的方式）

**模拟的 CDN 用法：**
```javascript
import OpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/index.js";
const converter = OpenCC.Converter({ from: "cn", to: "t" });
const result = await converter("简体中文");
```

**运行：**
```bash
node test/cdn-simple.mjs
```

**输出示例：**
```
🎯 测试高级 API（简化使用）

📝 创建简体转繁体转换器...

简体 → 繁体转换结果：
============================================================
输入: 开源中国转换工具
输出: 開源中國轉換工具
```

---

#### `cdn-usage.mjs`
测试**低级 WASM API**（完全控制）

**模拟的 CDN 用法：**
```javascript
import initOpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/opencc-wasm.js";
const wasmModule = await initOpenCC();
const api = {
  create: wasmModule.cwrap("opencc_create", "number", ["string"]),
  convert: wasmModule.cwrap("opencc_convert", "string", ["number", "string"]),
};
```

**运行：**
```bash
node test/cdn-usage.mjs
```

这个测试演示了完整的 WASM 模块操作流程：
1. 初始化 WASM 运行时
2. 包装 C API
3. 设置虚拟文件系统
4. 加载配置和字典文件
5. 创建转换器并执行转换
6. 清理资源

---

#### `cdn-test.html`
浏览器环境测试页面（带 UI）

**运行：**
```bash
# 启动 HTTP 服务器
python3 -m http.server 8888

# 访问
open http://localhost:8888/test/cdn-test.html
```

**功能：**
- 输入框：输入简体中文
- 转换按钮：点击转换
- 输出区域：显示繁体结果
- 状态提示：显示加载和转换状态

---

### 文档

#### `CDN_USAGE.md`
完整的 CDN 使用指南，包含：
- 布局结构说明
- 高级 API 使用方法
- 低级 WASM API 使用方法
- 浏览器和 Node.js 示例
- 性能优化建议
- 故障排查

---

## 快速开始

### 方式 1：运行所有测试
```bash
npm test                      # 运行核心功能测试（56个测试用例）
node test/cdn-simple.mjs      # 测试高级 API
node test/cdn-usage.mjs       # 测试低级 WASM API
```

### 方式 2：测试特定场景

**我想快速验证能否从 CDN 使用：**
```bash
node test/cdn-simple.mjs
```

**我需要了解底层 WASM 如何工作：**
```bash
node test/cdn-usage.mjs
```

**我想在浏览器中测试：**
```bash
python3 -m http.server 8888
# 访问 http://localhost:8888/test/cdn-test.html
```

---

## 测试原理

### CDN 模拟

测试文件通过导入本地 `dist/` 目录来模拟 CDN：

```javascript
// 测试中（本地）
import OpenCC from "../dist/esm/index.js";

// 实际生产（CDN）
import OpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/index.js";
```

这确保了测试环境与生产环境一致。

### 文件布局验证

测试验证了新的布局结构：
```
dist/
  esm/
    index.js              ← 高级 API 入口
    opencc-wasm.js        ← WASM glue code
    opencc-wasm.wasm      ← 必须在同一目录！
  cjs/
    index.cjs
    opencc-wasm.cjs
    opencc-wasm.wasm      ← 必须在同一目录！
```

关键点：**`.wasm` 文件必须与 `.js/.cjs` glue code 在同一目录**，这样 Emscripten 生成的加载代码才能正确找到 WASM 二进制文件。

---

## 添加新测试

### 创建新的 CDN 测试

```javascript
#!/usr/bin/env node
/**
 * 新测试：test/my-test.mjs
 */
import { join, dirname } from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = dirname(fileURLToPath(import.meta.url));
const distPath = join(__dirname, "../dist/esm/index.js");

const { default: OpenCC } = await import(distPath);

// 你的测试代码...
```

### 添加到测试脚本

在 `package.json` 中添加：
```json
{
  "scripts": {
    "test": "node --test test/opencc.test.js",
    "test:cdn": "node test/cdn-simple.mjs && node test/cdn-usage.mjs",
    "test:all": "npm test && npm run test:cdn"
  }
}
```

---

## 故障排查

### 测试失败：找不到 .wasm 文件

**错误信息：**
```
ENOENT: no such file or directory, open '.../opencc-wasm.wasm'
```

**解决方法：**
1. 确保已经运行 `npm run build`
2. 检查 `dist/esm/opencc-wasm.wasm` 是否存在
3. 重新运行 `node scripts/build-api.js`

### 测试失败：模块导入错误

**错误信息：**
```
Cannot find module '../dist/esm/index.js'
```

**解决方法：**
构建 dist 目录：
```bash
./build.sh && node scripts/build-api.js
```

---

## 相关资源

- [CDN_USAGE.md](./CDN_USAGE.md) - 完整使用指南
- [../README.md](../README.md) - 项目总览
- [测试用例数据](./testcases.json) - 56个测试用例的输入输出

---

## 测试覆盖

### 转换配置
✅ s2t, s2tw, s2hk, s2twp
✅ t2s, t2tw, t2hk, t2jp
✅ tw2s, tw2t, tw2sp
✅ hk2s, hk2t
✅ jp2t

### 使用场景
✅ Node.js ESM
✅ Node.js CommonJS
✅ 浏览器 (HTML)
✅ CDN 直接导入
✅ 自定义词典

### API 层次
✅ 高级 API (OpenCC.Converter)
✅ 工厂模式 (ConverterFactory)
✅ 自定义转换 (CustomConverter)
✅ 低级 WASM API (cwrap)
