# opencc-wasm

[![npm version](https://img.shields.io/npm/v/opencc-wasm.svg)](https://www.npmjs.com/package/opencc-wasm)
[![CDN](https://img.shields.io/badge/CDN-jsDelivr-orange.svg)](https://cdn.jsdelivr.net/npm/opencc-wasm@latest/dist/esm/index.js)
[![License](https://img.shields.io/badge/license-Apache--2.0-blue.svg)](LICENSE)

[English](README.md)

> 🚀 **開箱即用的中文簡繁轉換程式庫** - 3 行程式碼搞定，自動從 CDN 載入設定和字典！

OpenCC（Open Chinese Convert）的 WebAssembly 移植版本，完全相容原版 API。內建官方 OpenCC C++ 核心（透過 Emscripten 編譯），以及所有官方設定檔和預先建置的 `.ocd2` 字典檔。

**授權條款：** Apache-2.0

## ✨ 特色功能

- 🎯 **零設定** - 自動從 CDN 載入所有設定檔和字典檔
- 🔥 **3 行開始** - 最簡單的 API，匯入即用
- 🌐 **CDN 就緒** - 可直接從 jsDelivr/unpkg 使用，無需打包工具
- 📦 **一應俱全** - 包含所有 14+ 種官方轉換類型
- ⚡ **自動快取** - 資源首次載入後自動快取
- 🔧 **完全相容** - 相容 `opencc-js` API
- 🚫 **無需原生綁定** - 純 WASM，跨平台
- 💻 **通用支援** - 支援 Node.js、瀏覽器、Deno 等環境

## 🚀 快速開始

### 瀏覽器（CDN - 零安裝！）

```html
<script type="module">
  // 1. 從 CDN 匯入
  import OpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.3.0/dist/esm/index.js";

  // 2. 建立轉換器（自動下載所有資源！）
  const converter = OpenCC.Converter({ from: "cn", to: "tw" });

  // 3. 轉換 - 完成！
  const result = await converter("简体中文");
  console.log(result);  // 簡體中文
</script>
```

**就是這麼簡單！** 所有設定檔和字典檔都會自動從 CDN 下載。

### Node.js（NPM）

```bash
npm install opencc-wasm
```

```javascript
import OpenCC from "opencc-wasm";

const converter = OpenCC.Converter({ from: "cn", to: "tw" });
const result = await converter("简体中文");
console.log(result);  // 簡體中文
```

## 📖 API 參考

### OpenCC.Converter() - 建立轉換器

兩種方式指定轉換：

#### 方式 1：使用 `config` 參數（推薦）

直接指定 OpenCC 設定檔名稱：

```javascript
// 簡體 → 繁體（台灣慣用詞）
const converter = OpenCC.Converter({ config: "s2twp" });
const result = await converter("服务器软件");  // 伺服器軟體
```

**支援的設定檔：**

| 設定檔 | 說明 | 範例 |
|--------|------|------|
| `s2t` | 簡體 → 繁體 | 简体 → 簡體 |
| `s2tw` | 簡體 → 台灣正體 | 软件 → 軟件 |
| `s2twp` | 簡體 → 台灣正體（慣用詞） | 软件 → 軟體 |
| `s2hk` | 簡體 → 香港繁體 | 打印机 → 打印機 |
| `t2s` | 繁體 → 簡體 | 繁體 → 繁体 |
| `t2tw` | 繁體 → 台灣正體 | 台灣 → 臺灣 |
| `t2hk` | 繁體 → 香港繁體 | 香港 → 香港 |
| `t2jp` | 繁體 → 日文新字體 | 繁體 → 繁体 |
| `tw2s` | 台灣 → 簡體 | 軟體 → 软件 |
| `tw2sp` | 台灣 → 簡體（慣用詞） | 滑鼠 → 鼠标 |
| `tw2t` | 台灣 → 繁體 | 臺灣 → 台灣 |
| `hk2s` | 香港 → 簡體 | 打印機 → 打印机 |
| `hk2t` | 香港 → 繁體 | 香港 → 香港 |
| `jp2t` | 日文新字體 → 繁體 | 繁体 → 繁體 |
| `t2cngov` | 繁體 → 大陸政府標準繁體 | 潮溼 → 潮湿 |
| `t2cngov_keep_simp` | 繁體 → 大陸政府標準（保留簡體） | 简体繁體 → 简体繁體 |

#### 方式 2：使用 `from`/`to` 參數（傳統）

指定來源和目標語系：

```javascript
const converter = OpenCC.Converter({ from: "cn", to: "twp" });
const result = await converter("服务器");  // 伺服器
```

**語系代碼：**

| 代碼 | 說明 |
|------|------|
| `cn` | 簡體中文（中國大陸） |
| `tw` | 繁體中文（台灣） |
| `twp` | 台灣正體（含慣用詞） |
| `hk` | 繁體中文（香港） |
| `t` | 繁體中文（通用） |
| `s` | 簡體中文（別名） |
| `sp` | 簡體（含慣用詞） |
| `jp` | 日文新字體 |

**兩種方式功能完全相同！** 選擇您喜歡的即可。

### OpenCC.ConverterFactory() - 含自訂字典的轉換器

```javascript
const converter = OpenCC.ConverterFactory(
  "cn",        // from
  "tw",        // to
  [            // 自訂字典
    [["服务器", "伺服器"], ["文件", "檔案"]],
    "網路 网络 | 檔案 文件"
  ]
);

const result = await converter("服务器上的文件通过网络传输");
// 輸出：伺服器上的檔案通過網路傳輸
```

### OpenCC.CustomConverter() - 純自訂轉換器

```javascript
const converter = OpenCC.CustomConverter([
  [""", "「"],
  [""", "」"],
  ["'", "『"],
  ["'", "』"],
]);

const result = converter("这是"引号"和'单引号'");
// 輸出：这是「引号」和『单引号』
```

## 💡 使用範例

### React

```jsx
import { useState } from 'react';
import OpenCC from 'opencc-wasm';

function App() {
  const [output, setOutput] = useState('');

  const handleConvert = async () => {
    const converter = OpenCC.Converter({ config: "s2tw" });
    setOutput(await converter("简体中文"));
  };

  return (
    <div>
      <button onClick={handleConvert}>轉換</button>
      <div>{output}</div>
    </div>
  );
}
```

### Vue 3

```vue
<script setup>
import { ref } from 'vue';
import OpenCC from 'opencc-wasm';

const output = ref('');

async function handleConvert() {
  const converter = OpenCC.Converter({ config: "s2tw" });
  output.value = await converter("简体中文");
}
</script>

<template>
  <button @click="handleConvert">轉換</button>
  <div>{{ output }}</div>
</template>
```

### Node.js CLI 工具

```javascript
#!/usr/bin/env node
import OpenCC from 'opencc-wasm';

const text = process.argv[2] || "简体中文";
const converter = OpenCC.Converter({ config: "s2tw" });
console.log(await converter(text));
```

### Web Worker

```javascript
// worker.js
import OpenCC from 'opencc-wasm';

let converters = {};

self.onmessage = async (e) => {
  const { config, text } = e.data;

  if (!converters[config]) {
    converters[config] = OpenCC.Converter({ config });
  }

  const result = await converters[config](text);
  self.postMessage(result);
};
```

```javascript
// main.js
const worker = new Worker('worker.js', { type: 'module' });

worker.onmessage = (e) => {
  console.log('結果：', e.data);
};

worker.postMessage({ config: 's2tw', text: '简体中文' });
```

## 🔧 最佳實務

### ✅ 重複使用轉換器實例

```javascript
// ✅ 好：建立一次，多次使用
const converter = OpenCC.Converter({ config: "s2tw" });

for (const text of manyTexts) {
  await converter(text);  // 快！
}
```

```javascript
// ❌ 避免：每次都建立新實例
for (const text of manyTexts) {
  const converter = OpenCC.Converter({ config: "s2tw" });  // 慢！
  await converter(text);
}
```

### 多個轉換器（自動快取）

```javascript
// 建立多個轉換器（資源自動快取）
const s2t = OpenCC.Converter({ config: "s2t" });
const s2tw = OpenCC.Converter({ config: "s2tw" });
const t2s = OpenCC.Converter({ config: "t2s" });

// 獨立使用
console.log(await s2t("简体"));   // 簡體
console.log(await s2tw("软件"));  // 軟體
console.log(await t2s("繁體"));   // 繁体
```

### TypeScript

```typescript
import OpenCC from 'opencc-wasm';

type ConfigName = 's2t' | 's2tw' | 's2twp' | 't2s';

async function convert(config: ConfigName, text: string): Promise<string> {
  const converter = OpenCC.Converter({ config });
  return await converter(text);
}

const result = await convert('s2tw', '简体中文');
```

## 🏗️ 建置

專案使用兩階段建置流程：

### 階段 1：建置 WASM

```bash
./build.sh
```

編譯 OpenCC + marisa-trie 成 WASM，輸出至 `build/`：
- `build/opencc-wasm.esm.js` - ESM WASM 膠合程式
- `build/opencc-wasm.cjs` - CJS WASM 膠合程式
- `build/opencc-wasm.wasm` - WASM 二進位檔

### 階段 2：建置 API

```bash
node scripts/build-api.js
```

產生可發佈的發行版至 `dist/`：
- 複製 WASM 檔案至 `dist/esm/` 和 `dist/cjs/`
- 轉換原始碼至生產環境路徑
- 複製資料檔至 `dist/data/`

### 完整建置

```bash
npm run build
```

自動執行兩個階段。

## 🧪 測試

```bash
npm test
```

執行上游 OpenCC 測試案例來驗證 WASM 建置。

## 📁 專案結構

```
wasm-lib/
├── build/              ← 中間產物（gitignored）
├── dist/               ← 可發佈版本（已提交）
│   ├── esm/
│   │   ├── index.js
│   │   ├── opencc-wasm.js
│   │   └── opencc-wasm.wasm
│   ├── cjs/
│   │   ├── index.cjs
│   │   ├── opencc-wasm.cjs
│   │   └── opencc-wasm.wasm
│   └── data/           ← OpenCC 設定檔 + 字典
├── index.js            ← 原始碼 API
├── index.d.ts          ← TypeScript 型別定義
└── scripts/
    └── build-api.js    ← 建置腳本
```

## ❓ 常見問題

**Q：設定檔和字典會自動載入嗎？還是需要我手動下載？**

A：自動載入！高階 API（`OpenCC.Converter()`）會自動從 CDN 下載所有需要的檔案。

**Q：每次轉換都會重新下載嗎？**

A：不會！資源在首次載入後會快取起來。

**Q：可以離線使用嗎？**

A：可以！透過 npm 安裝時，所有資源都已包含在套件中。瀏覽器環境可使用 Service Worker 實現離線快取。

**Q：應該用 `config` 還是 `from`/`to` 參數？**

A：兩者功能完全相同。如果您熟悉 OpenCC 設定檔名稱，用 `config`；若偏好語系導向的方式，用 `from`/`to`。

**Q：為什麼第一次轉換比較慢？**

A：首次載入需要下載設定檔和字典檔（約 1-2MB）。後續轉換會很快（已快取）。

## 📝 注意事項

- 使用持久的 OpenCC 控制代碼避免重複載入設定
- 字典儲存在虛擬檔案系統的 `/data/dict/` 中
- 記憶體按需成長（`ALLOW_MEMORY_GROWTH=1`）
- 效能：專注於精確度和與官方 OpenCC 的相容性。原始吞吐量可能比純 JavaScript 實作慢，但保證完整的 OpenCC 行為。

## 📜 變更歷史

### 0.3.0 - 2026-01-03

**🚨 重大變更：新的發行版佈局**

`.wasm` 檔案已移至與膠合程式同目錄：
- `dist/esm/opencc-wasm.wasm`（舊：`dist/opencc-wasm.esm.wasm`）
- `dist/cjs/opencc-wasm.wasm`（舊：`dist/opencc-wasm.cjs.wasm`）

**新增：**
- CDN 支援，可直接在瀏覽器中使用
- 完整測試套件
- 自動載入設定檔和字典檔

### 0.2.1

- 提供兩種 wasm 檔名以相容性

### 0.2.0

- 從 OpenCC commit [`36c7cbbc`](https://github.com/frankslin/OpenCC/commit/36c7cbbc9702d2a46a89ea7a55ff8ba5656455df) 重新建置
- 新的 dist 佈局，ESM/CJS 分離
- 測試改用 `node:test` 重寫

---

**用 ❤️ 為中文 NLP 社群打造**
