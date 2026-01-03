# 测试指南

## 快速开始

```bash
npm test          # 运行所有测试（推荐）
```

## 测试命令说明

### `npm test` - 运行所有测试 ⭐
运行完整的测试套件，包括：
- ✅ 56 个核心功能测试（所有转换配置）
- ✅ 高级 API CDN 使用测试
- ✅ 低级 WASM API 使用测试

**输出示例：**
```
> opencc-wasm@0.2.1 test
> npm run test:core && npm run test:cdn

[核心测试]
# tests 56
# pass 56
# fail 0

[CDN 高级 API 测试]
🎯 测试高级 API（简化使用）
✅ 测试完成！

[CDN 低级 API 测试]
🧪 测试 CDN 风格的使用方式
🎉 所有测试通过！
```

---

### `npm run test:core` - 只运行核心功能测试
测试所有 56 个转换配置用例。

**命令：**
```bash
npm run test:core
```

**测试内容：**
- s2t, s2tw, s2hk, s2twp（简体→繁体/台湾/香港）
- t2s, t2tw, t2hk, t2jp（繁体→简体/台湾/香港/日文）
- tw2s, tw2t, tw2sp（台湾→简体/繁体）
- hk2s, hk2t（香港→简体/繁体）
- jp2t（日文→繁体）

---

### `npm run test:cdn` - 只运行 CDN 使用测试
测试从 CDN 导入和使用的场景。

**命令：**
```bash
npm run test:cdn
```

**测试内容：**
1. **高级 API 测试** (`cdn-simple.mjs`)
   - 模拟 `import OpenCC from "CDN/dist/esm/index.js"`
   - 测试 `OpenCC.Converter()` API
   - 验证简体→繁体转换

2. **低级 WASM API 测试** (`cdn-usage.mjs`)
   - 模拟 `import initOpenCC from "CDN/dist/esm/opencc-wasm.js"`
   - 测试完整的 WASM 初始化流程
   - 验证手动配置和字典加载
   - 测试 C API 包装

---

## 单独运行测试

### 运行单个 CDN 测试

```bash
# 高级 API 测试（最常用场景）
node test/cdn-simple.mjs

# 低级 WASM API 测试（完整流程）
node test/cdn-usage.mjs
```

### 运行浏览器测试

```bash
# 启动 HTTP 服务器
python3 -m http.server 8888

# 访问测试页面
open http://localhost:8888/test/cdn-test.html
```

---

## 测试文件结构

```
test/
├── opencc.test.js       # 核心功能测试（56个用例）
├── cdn-simple.mjs       # CDN 高级 API 测试
├── cdn-usage.mjs        # CDN 低级 WASM API 测试
├── cdn-test.html        # 浏览器环境测试
├── testcases.json       # 测试用例数据
├── README.md            # 测试套件总览
├── TESTING.md           # 本文件
└── CDN_USAGE.md         # CDN 使用完整指南
```

---

## 测试方法详解

### 1. 核心功能测试 (test:core)

**文件：** `opencc.test.js`

**工作原理：**
1. 从 `testcases.json` 加载 56 个测试用例
2. 对每个用例：
   - 创建对应的转换器（如 s2t, tw2s 等）
   - 输入测试文本
   - 验证输出是否与期望匹配

**测试用例格式：**
```json
{
  "config": "s2t.json",
  "case_id": "case_001",
  "input": "开源中国",
  "expected": "開源中國"
}
```

---

### 2. CDN 高级 API 测试 (cdn-simple.mjs)

**模拟场景：**
```javascript
// 用户从 CDN 导入
import OpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/index.js";

// 创建转换器
const converter = OpenCC.Converter({ from: "cn", to: "t" });

// 转换文本
const result = await converter("简体中文");
```

**测试流程：**
1. 导入本地 `dist/esm/index.js`（模拟 CDN）
2. 调用 `OpenCC.Converter({ from: "cn", to: "t" })`
3. 测试 3 个文本转换
4. 验证输出正确性

**验证点：**
- ✅ ES 模块导入成功
- ✅ API 接口可用
- ✅ 转换结果正确
- ✅ 异步调用正常

---

### 3. CDN 低级 WASM API 测试 (cdn-usage.mjs)

**模拟场景：**
```javascript
// 直接导入 WASM 模块
import initOpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/opencc-wasm.js";

// 初始化并手动操作
const wasmModule = await initOpenCC();
const api = {
  create: wasmModule.cwrap("opencc_create", "number", ["string"]),
  convert: wasmModule.cwrap("opencc_convert", "string", ["number", "string"]),
};
```

**测试流程：**
1. 导入 WASM glue code
2. 初始化 WASM 运行时
3. 包装 C API 函数
4. 设置虚拟文件系统（FS）
5. 加载配置文件（s2t.json）
6. 收集并加载字典文件（.ocd2）
7. 修改配置路径
8. 创建转换器实例
9. 执行转换
10. 清理资源

**验证点：**
- ✅ WASM 模块加载成功
- ✅ C API 包装正确
- ✅ 虚拟文件系统可用
- ✅ 配置和字典加载成功
- ✅ 转换功能正常
- ✅ 资源清理完成

---

## 关键测试验证

### 1. 文件布局验证

测试确保 `.wasm` 文件与 glue code 在同一目录：

```
dist/esm/opencc-wasm.js    ← glue code
dist/esm/opencc-wasm.wasm  ← 必须在同一目录！
```

**为什么重要？**
Emscripten 生成的 glue code 默认在同目录查找 `.wasm` 文件。

---

### 2. CDN 兼容性验证

测试验证了以下 CDN 使用模式都能正常工作：

```javascript
// ✅ 高级 API（推荐）
import OpenCC from "CDN/dist/esm/index.js";

// ✅ 低级 WASM API（高级用户）
import initOpenCC from "CDN/dist/esm/opencc-wasm.js";

// ✅ CommonJS（Node.js）
const OpenCC = require("CDN/dist/cjs/index.cjs");

// ✅ 直接导出 WASM 模块
import wasmGlue from "CDN/dist/esm/opencc-wasm.js";
```

---

### 3. API 兼容性验证

测试涵盖了所有 API 层次：

```javascript
// ✅ 简化 API
OpenCC.Converter({ from: "cn", to: "t" })

// ✅ 工厂模式
OpenCC.ConverterFactory("cn", "tw", [customDict])

// ✅ 自定义转换器
OpenCC.CustomConverter([["词", "詞"]])

// ✅ 直接 WASM API
wasmModule.cwrap("opencc_convert", ...)
```

---

## 持续集成（CI）

在 CI 环境中运行测试：

```yaml
# .github/workflows/test.yml
- name: Run tests
  run: |
    cd wasm-lib
    npm test
```

**预期输出：**
```
✅ 56/56 core tests passed
✅ CDN high-level API test passed
✅ CDN low-level WASM API test passed
```

---

## 故障排查

### 问题：测试找不到 dist/ 文件

**症状：**
```
Error: Cannot find module '../dist/esm/index.js'
```

**解决：**
```bash
npm run build
```

---

### 问题：WASM 文件加载失败

**症状：**
```
ENOENT: no such file or directory, open '.../opencc-wasm.wasm'
```

**检查：**
```bash
ls -la dist/esm/opencc-wasm.wasm
ls -la dist/cjs/opencc-wasm.wasm
```

**修复：**
```bash
node scripts/build-api.js
```

---

### 问题：某些测试失败

**调试步骤：**
```bash
# 1. 只运行核心测试
npm run test:core

# 2. 只运行 CDN 测试
npm run test:cdn

# 3. 运行单个 CDN 测试
node test/cdn-simple.mjs
```

---

## 添加新测试

### 添加核心功能测试用例

编辑 `testcases.json`：
```json
{
  "config": "s2t.json",
  "case_id": "case_057",
  "input": "新测试",
  "expected": "新測試"
}
```

---

### 添加 CDN 测试

创建新文件 `test/cdn-custom.mjs`：
```javascript
#!/usr/bin/env node
import { join, dirname } from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = dirname(fileURLToPath(import.meta.url));
const { default: OpenCC } = await import(join(__dirname, "../dist/esm/index.js"));

// 你的测试逻辑...
```

添加到 `package.json`：
```json
"test:cdn": "node test/cdn-simple.mjs && node test/cdn-usage.mjs && node test/cdn-custom.mjs"
```

---

## 性能测试

虽然不在标准测试套件中，但可以手动测试性能：

```javascript
// test/performance.mjs
const start = Date.now();
const result = await converter(longText);
console.log(`转换 ${longText.length} 字符耗时: ${Date.now() - start}ms`);
```

---

## 测试覆盖率

当前测试覆盖：

| 类别 | 覆盖率 |
|------|--------|
| 转换配置 | 100% (15/15) |
| API 层次 | 100% (4/4) |
| 使用场景 | 100% (Node ESM/CJS, 浏览器, CDN) |
| 错误处理 | 基本覆盖 |

---

## 相关资源

- **README.md** - 测试套件总览
- **CDN_USAGE.md** - CDN 使用完整指南
- **../README.md** - 项目主文档
