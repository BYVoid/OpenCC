# OpenCC for Node.js

OpenCC 提供高品質的簡體中文、繁體中文與地區用字轉換。本套件包含
OpenCC 原生函式庫的官方 Node.js 綁定。

## 安裝

```bash
npm install opencc
```

本套件需要 Node.js `>=20.17`。

OpenCC 以預編譯二進位檔提供原生綁定。安裝時會自動選取符合目前平台的
`@opencc/opencc-<platform>-<arch>` 套件，涵蓋 macOS（x64/arm64）、
Linux（x64/arm64）與 Windows（x64）。沒有預編譯二進位套件的平台，
`npm install` 會以 Bazel 從原始碼構建：編譯 addon 並重新生成字典。
這需要可用的 C++ 工具鏈與網路；`bazel` 或 `bazelisk` 取自 PATH，
找不到時會透過 `npx` 自動下載 bazelisk，字典生成腳本所需的 Python
由 Bazel 自動下載 hermetic 工具鏈，無需系統安裝。

## 基本用法

ES modules：

```js
import OpenCC from 'opencc';

const converter = new OpenCC('s2t.json');
console.log(converter.convertSync('汉字'));
```

Promise API：

```js
import OpenCC from 'opencc';

const converter = new OpenCC('s2tw.json');
console.log(await converter.convertPromise('鼠标和软件'));
```

具名匯入：

```js
import { OpenCC } from 'opencc';

const converter = new OpenCC('t2s.json');
console.log(converter.convertSync('漢字'));
```

較舊的專案仍可使用 CommonJS：

```js
const { OpenCC } = require('opencc');

const converter = new OpenCC('s2t.json');
console.log(converter.convertSync('汉字'));
```

Callback API：

```js
import OpenCC from 'opencc';

const converter = new OpenCC('t2s.json');

converter.convert('漢字', (err, text) => {
  if (err) throw err;
  console.log(text);
});
```

套件內已包含 TypeScript 型別宣告。

## API

### `new OpenCC(config?, options?)`

建立轉換器。若省略 `config`，OpenCC 會使用 `s2t.json`。

`config` 可以是內建配置檔名稱，例如 `s2t.json`，也可以是自訂
OpenCC 配置檔的絕對路徑，或 OpenCC 配置物件。

`options.includeTofuRiskDictionaries` 控制是否載入標記為可能輸出 tofu
的字典。這裡的 tofu 指無法顯示的中文字，有時會被渲染為方塊，俗稱
豆腐塊。為維持 JavaScript API 相容性，預設值為 `true`。

`options.configDirectory` 設定 inline 配置物件中相對字典路徑的基準
目錄。預設會使用套件內建的 OpenCC assets 目錄。

### `OpenCC.fromConfig(config, options?)`

從 OpenCC 配置物件建立轉換器。這等同於將該物件傳給
`new OpenCC(config, options)`。

### `converter.convertSync(input)`

同步轉換文字，並回傳轉換後的字串。

### `converter.convert(input, callback)`

非同步轉換文字。

Callback 會收到 `(err, convertedText)`。

### `converter.convertPromise(input)`

非同步轉換文字，並回傳 `Promise<string>`。

### `OpenCC.version`

內建 OpenCC 原生函式庫的版本字串。

### `OpenCC.generateDict(inputFileName, outputFileName, formatFrom, formatTo)`

從另一種字典格式產生字典檔。

例如，將文字字典轉換為 OpenCC 的二進位 `ocd2` 格式：

```js
import { OpenCC } from 'opencc';

OpenCC.generateDict('my-dict.txt', 'my-dict.ocd2', 'text', 'ocd2');
```

## 命令列

若要使用 npm 命令列工具，請全域安裝 OpenCC：

```bash
npm install -g opencc
```

本套件會安裝 `opencc` 命令：

```bash
opencc -c s2t.json -i input.txt -o output.txt
```

也可以從 stdin 讀取，並輸出到 stdout：

```bash
echo '汉字' | opencc -c s2t.json
```

選項：

```text
-c, --config <file>  配置檔。預設為 s2t.json。
-i, --input <file>   從 <file> 讀取原文。預設為 stdin。
-o, --output <file>  將轉換結果寫入 <file>。預設為 stdout。
--include-tofu-risk-dictionaries
                     載入可能輸出 tofu 的字典；tofu 指無法顯示時
                     渲染成方塊的中文字。
-v, --version        印出 OpenCC 版本。
-h, --help           印出說明。
```

npm CLI 預設會略過標記為可能輸出 tofu 的字典。這裡的 tofu 指無法
顯示的中文字，有時會被渲染為方塊，俗稱豆腐塊；若要載入，請使用
`--include-tofu-risk-dictionaries`。npm CLI 僅支援文字轉換。若需要
inspect、segmentation 輸出、自訂資源搜尋路徑，或其他進階 CLI 功能，
請使用原生 OpenCC 命令列工具。

## 內建配置

| 配置檔 | 轉換 |
| --- | --- |
| `s2t.json` | Simplified Chinese to Traditional Chinese (OpenCC Standard) / 簡體到 OpenCC 標準繁體 |
| `t2s.json` | Traditional Chinese (OpenCC Standard) to Simplified Chinese / OpenCC 標準繁體到簡體 |
| `s2tw.json` | Simplified Chinese to Traditional Chinese (Taiwan Standard) / 簡體到台灣正體 |
| `tw2s.json` | Traditional Chinese (Taiwan Standard) to Simplified Chinese / 台灣正體到簡體 |
| `s2hk.json` | Simplified Chinese to Traditional Chinese (Hong Kong variant) / 簡體到香港繁體 |
| `hk2s.json` | Traditional Chinese (Hong Kong variant) to Simplified Chinese / 香港繁體到簡體 |
| `s2twp.json` | Simplified Chinese to Traditional Chinese (Taiwan Standard, with Taiwan Phrases) / 簡體到台灣正體（含台灣常用詞彙） |
| `tw2sp.json` | Traditional Chinese (Taiwan Standard) to Simplified Chinese (Mainland China Phrases) / 台灣正體到簡體（含中國大陸常用詞彙） |
| `t2tw.json` | Traditional Chinese (OpenCC Standard) to Traditional Chinese (Taiwan Standard) / OpenCC 標準繁體到台灣正體 |
| `tw2t.json` | Traditional Chinese (Taiwan Standard) to Traditional Chinese (OpenCC Standard) / 台灣正體到 OpenCC 標準繁體 |
| `t2hk.json` | Traditional Chinese (OpenCC Standard) to Traditional Chinese (Hong Kong variant) / OpenCC 標準繁體到香港繁體 |
| `hk2t.json` | Traditional Chinese (Hong Kong variant) to Traditional Chinese (OpenCC Standard) / 香港繁體到 OpenCC 標準繁體 |
| `t2jp.json` | Traditional Chinese Characters (Kyūjitai) to New Japanese Kanji (Shinjitai) / OpenCC 標準繁體（日文舊字體）到日文新字體 |
| `jp2t.json` | New Japanese Kanji (Shinjitai) to Traditional Chinese Characters (Kyūjitai) / 日文新字體到 OpenCC 標準繁體（日文舊字體） |

## 自訂配置

傳入自訂 OpenCC 配置 JSON 檔案的路徑：

```js
import OpenCC from 'opencc';

const converter = new OpenCC('/absolute/path/to/custom.json');
console.log(converter.convertSync('汉字'));
```

CLI 的相對配置路徑會從目前工作目錄解析。在 JavaScript API 中，相對
配置名稱會從套件 assets 目錄解析，因此自訂配置應使用絕對路徑傳入。

也可以直接傳入 OpenCC 配置物件：

```js
import OpenCC from 'opencc';

const converter = OpenCC.fromConfig({
  name: 'Custom Inline Config',
  segmentation: {
    type: 'mmseg',
    dict: { type: 'inline', entries: { '鼠标': '鼠标' } },
  },
  conversion_chain: [{
    dict: { type: 'inline', entries: { '鼠标': '滑鼠' } },
  }],
});

console.log(converter.convertSync('鼠标坏了'));
```

若 inline 配置引用相對字典檔，請傳入 `configDirectory`：

```js
const converter = OpenCC.fromConfig(config, {
  configDirectory: '/absolute/path/to/opencc-resources',
});
```

自訂字典可以透過 `OpenCC.generateDict()` 產生，也可以使用完整 OpenCC
發行版中的原生 `opencc_dict` 工具。

## 可選 Jieba 配置

部分 OpenCC 配置會使用 jieba 分詞。安裝可選的 `opencc-jieba` 套件後，
即可使用 jieba 支援的配置：

```bash
npm install opencc opencc-jieba
```

安裝 `opencc-jieba` 後，JavaScript API 與 npm `opencc` CLI 可以自動
載入其中的配置：

可用的插件配置包含 `s2t_jieba.json`、`s2tw_jieba.json`、
`s2hk_jieba.json`、`s2twp_jieba.json` 與 `tw2sp_jieba.json`。

```js
import OpenCC from 'opencc';

const converter = new OpenCC('s2twp_jieba');
console.log(converter.convertSync('软件鼠标'));
```

npm CLI 也可使用相同配置名稱進行文字轉換：

```bash
opencc -c s2twp_jieba
```

`--inspect`、`--segmentation` 等診斷模式仍請使用原生 OpenCC CLI。

## 相關 npm 套件

[`opencc`](https://www.npmjs.com/package/opencc) npm 套件是官方 OpenCC
C++ 專案的 Node.js 原生綁定。它面向 Node.js，依賴原生或預編譯
二進位檔，並遵循官方 OpenCC 引擎。當官方 OpenCC 配置與執行環境支援
時，它可以使用 Jieba 等擴充分詞演算法。

[`opencc-js`](https://www.npmjs.com/package/opencc-js) npm 套件是可在
瀏覽器與 Node.js 使用的純 JavaScript 實作。它會打包由 `opencc-data`
產生的字典資料，因此不需要原生二進位檔，也不會在瀏覽器執行時額外抓取
文字字典檔。

`opencc-js` 已將轉換流程對齊官方 OpenCC 實作，包含內建轉換器的詞彙
分詞，並以 upstream OpenCC 測試案例與 golden outputs 測試。不過它仍
不應被視為能保證對所有可能輸入都產生 100% 相同的結果。

`opencc-js` 目前支援其內建轉換器使用的 OpenCC mmseg-style 分詞，但不
支援 Jieba 等擴充分詞演算法。

[`opencc-wasm`](https://www.npmjs.com/package/opencc-wasm) npm 套件是
另一個可在瀏覽器使用的實作。它使用 WebAssembly，配置與轉換邏輯和官方
`opencc` 套件保持一致，並可透過官方 OpenCC runtime 支援 Jieba 分詞。

## 開發

從 repository root 執行：

```bash
npm install
npm test
```

若要為發布建置預編譯原生 addon，請參考
[`node/PUBLISHING.md`](./PUBLISHING.md)。
