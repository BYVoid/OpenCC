# Open Chinese Convert 開放中文轉換

[![CMake](https://github.com/BYVoid/OpenCC/actions/workflows/cmake.yml/badge.svg)](https://github.com/BYVoid/OpenCC/actions/workflows/cmake.yml)
[![Bazel](https://github.com/BYVoid/OpenCC/actions/workflows/bazel.yml/badge.svg)](https://github.com/BYVoid/OpenCC/actions/workflows/bazel.yml)
[![MSVC](https://github.com/BYVoid/OpenCC/actions/workflows/msvc.yml/badge.svg)](https://github.com/BYVoid/OpenCC/actions/workflows/msvc.yml)
[![Node.js CI](https://github.com/BYVoid/OpenCC/actions/workflows/nodejs.yml/badge.svg)](https://github.com/BYVoid/OpenCC/actions/workflows/nodejs.yml)
[![Python CI](https://github.com/BYVoid/OpenCC/actions/workflows/python.yml/badge.svg)](https://github.com/BYVoid/OpenCC/actions/workflows/python.yml)
[![AppVeyor](https://img.shields.io/appveyor/ci/Carbo/OpenCC.svg)](https://ci.appveyor.com/project/Carbo/OpenCC)

[![latest packaged version(s)](https://repology.org/badge/latest-versions/opencc.svg)](https://repology.org/project/opencc/versions)

## Introduction 介紹

![OpenCC](https://opencc.byvoid.com/img/opencc.png)

Open Chinese Convert (OpenCC, 開放中文轉換) is an opensource project for conversions between Traditional Chinese, Simplified Chinese and Japanese Kanji (Shinjitai). It supports character-level and phrase-level conversion, character variant conversion and regional idioms among Mainland China, Taiwan and Hong Kong. This is not translation tool between Mandarin and Cantonese, etc.

中文簡繁轉換開源項目，支持詞彙級別的轉換、異體字轉換和地區習慣用詞轉換（中國大陸、台灣、香港、日本新字體）。不提供普通話與粵語的轉換。

Discussion (Telegram): https://t.me/open_chinese_convert

### Features 特點

* 嚴格區分「一簡對多繁」和「一簡對多異」。
* 完全兼容異體字，可以實現動態替換。
* 嚴格審校一簡對多繁詞條，原則爲「能分則不合」。
* 支持中國大陸、台灣、香港異體字和地區習慣用詞轉換，如「裏」「裡」、「鼠標」「滑鼠」。
* 詞庫和函數庫完全分離，可以自由修改、導入、擴展。

## Installation 安裝

### Package Managers 包管理器

* [Debian](https://tracker.debian.org/pkg/opencc)
* [Ubuntu](https://launchpad.net/ubuntu/+source/opencc)
* [Fedora](https://packages.fedoraproject.org/pkgs/opencc/opencc/)
* [Arch Linux](https://archlinux.org/packages/extra/x86_64/opencc/)
* [macOS (Homebrew)](https://formulae.brew.sh/formula/opencc)
* WinGet (使用 `winget install BYVoid.OpenCC` 命令)
* [Bazel](https://registry.bazel.build/modules/opencc)
* [Node.js](https://npmjs.org/package/opencc)
* [Python](https://pypi.org/project/OpenCC/)
* [More (Repology)](https://repology.org/project/opencc/versions)

### Prebuilt 預編譯

* Windows (x86_64): [OpenCC-1.3.0](https://github.com/BYVoid/OpenCC/releases/download/ver.1.3.0/OpenCC-1.3.0-windows-x64-portable.zip) ([SHA-256](https://github.com/BYVoid/OpenCC/releases/download/ver.1.3.0/OpenCC-1.3.0-windows-x64-portable.zip.sha256))
    This is a Windows release intended for WinGet distribution. For details, see [doc/windows-winget-release.md](doc/windows-winget-release.md).
* Debian/Ubuntu (amd64):
    * [opencc_1.3.0_amd64.deb](https://github.com/BYVoid/OpenCC/releases/download/ver.1.3.0/opencc_1.3.0_amd64.deb)
    * [opencc-jieba_1.3.0_amd64.deb](https://github.com/BYVoid/OpenCC/releases/download/ver.1.3.0/opencc-jieba_1.3.0_amd64.deb)

## Usage 使用

### Online 線上轉換

https://opencc.js.org/converter?config=s2t

### Node.js

`npm install opencc`

The npm package supports Node.js `>=20.17 <26`. It uses bundled Node-API
prebuilds when available and falls back to a local `node-gyp` build when the
current platform does not have a matching prebuild.

To install the npm CLI:

```sh
npm install -g opencc
opencc -c s2t.json -i input.txt -o output.txt
```

The npm CLI supports basic text conversion. Plugins, `--inspect`, and
`--segmentation` require the native OpenCC CLI.

```ts
import { OpenCC } from 'opencc';
async function main() {
  const converter: OpenCC = new OpenCC('s2t.json');
  const result: string = await converter.convertPromise('汉字');
  console.log(result);  // 漢字
}
```

See [demo.js](https://github.com/BYVoid/OpenCC/blob/master/node/demo.js) and [ts-demo.ts](https://github.com/BYVoid/OpenCC/blob/master/node/ts-demo.ts).

### Python

`pip install opencc` (Windows, Linux, macOS)

```python
import opencc
converter = opencc.OpenCC('s2t.json')
converter.convert('汉字')  # 漢字
```

### C++

```c++
#include "opencc.h"

int main() {
  const opencc::SimpleConverter converter("s2t.json");
  converter.Convert("汉字");  // 漢字
  return 0;
}
```

[Full example with Bazel](https://github.com/BYVoid/opencc-bazel-example)

### C

```c
#include "opencc.h"

int main() {
  opencc_t opencc = opencc_open("s2t.json");
  const char* input = "汉字";
  char* converted = opencc_convert_utf8(opencc, input, strlen(input));  // 漢字
  opencc_convert_utf8_free(converted);
  opencc_close(opencc);
  return 0;
}

```

[Full Document 完整文檔](https://opencc.byvoid.com/docs/)

### Command Line

* `opencc --help`
* `opencc_dict --help`

#### Segmentation and Inspection Modes

OpenCC CLI supports two diagnostic modes that output JSON instead of converted text:

**`--segmentation`** — Output segmentation result only (no conversion):

```bash
echo "他只看了几行日志，就一叶知秋，猜到整个系统是数据库连接池出了问题" | opencc -c s2twp.json --segmentation
# {"input":"他只看了几行日志，就一叶知秋，猜到整个系统是数据库连接池出了问题","segments":["他","只看","了几行","日志","，就","一叶知秋","，猜到","整个","系统","是","数据库","连接池","出了","问题"]}
```

**`--inspect`** — Output full inspection result (segmentation + per-stage conversion + final output):

```bash
echo "他只看了几行日志，就一叶知秋，猜到整个系统是数据库连接池出了问题" | opencc -c s2twp.json --inspect
# {"input":"他只看了几行日志，就一叶知秋，猜到整个系统是数据库连接池出了问题","segments":["他","只看","了几行","日志","，就","一叶知秋","，猜到","整个","系统","是","数据库","连接池","出了","问题"],"stages":[{"index":1,"segments":["他","只看","了幾行","日誌","，就","一葉知秋","，猜到","整個","系統","是","數據庫","連接池","出了","問題"]},{"index":2,"segments":["他","只看","了幾行","日誌","，就","一葉知秋","，猜到","整個","系統","是","資料庫","連線池","出了","問題"]},{"index":3,"segments":["他","只看","了幾行","日誌","，就","一葉知秋","，猜到","整個","系統","是","資料庫","連線池","出了","問題"]}],"output":"他只看了幾行日誌，就一葉知秋，猜到整個系統是資料庫連線池出了問題"}

# Pretty-print with jq:
echo "他只看了几行日志，就一叶知秋，猜到整个系统是数据库连接池出了问题" | opencc -c s2twp.json --inspect | jq .
```

These modes are useful for diagnosing conversion issues:

1. Use `--segmentation` to verify that the input is segmented as expected.
2. Use `--inspect` to see which conversion stage produces an unexpected result.

Rules:
- `--segmentation` and `--inspect` are mutually exclusive.

### Other Ports (Unofficial)

* Swift (iOS): [SwiftyOpenCC](https://github.com/XQS6LB3A/SwiftyOpenCC)
* iOSOpenCC (pod): [iOSOpenCC](https://github.com/swiftdo/OpenCC)
* Java: [opencc4j](https://github.com/houbb/opencc4j)
* Android: [android-opencc](https://github.com/qichuan/android-opencc)
* PHP: [opencc4php](https://github.com/nauxliu/opencc4php)
* Pure JavaScript: [opencc-js](https://github.com/nk2028/opencc-js)
* WebAssembly:
    * [opencc-wasm](https://www.npmjs.com/package/opencc-wasm) ([website](https://opencc.js.org/))
    * [wasm-opencc](https://github.com/oyyd/wasm-opencc)
* Browser Extension: [opencc-extension](https://github.com/tnychn/opencc-extension)
* Go (Pure): [OpenCC for Go](https://github.com/longbridge/opencc)
* Dart (native-assets): [opencc-dart](https://github.com/lindeer/opencc-dart)

### Configurations 配置文件

#### 預設配置文件

* `s2t.json` **Simplified Chinese** to **Traditional Chinese (OpenCC Standard)** / **簡體** 到 **OpenCC 標準繁體**
* `t2s.json` **Traditional Chinese (OpenCC Standard)** to **Simplified Chinese** / **OpenCC 標準繁體** 到 **簡體**
* `s2tw.json` **Simplified Chinese** to **Traditional Chinese (Taiwan Standard)** / **簡體** 到 **台灣正體**
* `tw2s.json` **Traditional Chinese (Taiwan Standard)** to **Simplified Chinese** / **台灣正體** 到 **簡體**
* `s2hk.json` **Simplified Chinese** to **Traditional Chinese (Hong Kong variant)** / **簡體** 到 **香港繁體**
* `hk2s.json` **Traditional Chinese (Hong Kong variant)** to **Simplified Chinese** / **香港繁體** 到 **簡體**
* `s2twp.json` **Simplified Chinese** to **Traditional Chinese (Taiwan Standard)** with Taiwanese idiom / **簡體** 到 **台灣正體** 並轉換爲台灣常用詞彙
* `tw2sp.json` **Traditional Chinese (Taiwan Standard)** to **Simplified Chinese** with Mainland Chinese idiom / **台灣正體** 到 **簡體** 並轉換爲中國大陸常用詞彙
* `t2tw.json` **Traditional Chinese (OpenCC Standard)** to **Traditional Chinese (Taiwan Standard)** / **OpenCC 標準繁體** 到 **台灣正體**
* `tw2t.json` **Traditional Chinese (Taiwan standard)** to **Traditional Chinese (OpenCC Standard)** / **台灣正體** 到 **OpenCC 標準繁體**
* `t2hk.json` **Traditional Chinese (OpenCC Standard)** to **Traditional Chinese (Hong Kong variant)** / **OpenCC 標準繁體** 到 **香港繁體**
* `hk2t.json` **Traditional Chinese (Hong Kong variant)** to **Traditional Chinese (OpenCC Standard)** / **香港繁體** 到 **OpenCC 標準繁體**
* `t2jp.json` **Traditional Chinese Characters (Kyūjitai)** to **New Japanese Kanji (Shinjitai)** / **OpenCC 標準繁體（日文舊字體）** 到 **日文新字體**
* `jp2t.json` **New Japanese Kanji (Shinjitai)** to **Traditional Chinese Characters (Kyūjitai)** / **日文新字體** 到 **OpenCC 標準繁體（日文舊字體）**

#### 指定配置文件

通过环境变量`OPENCC_DATA_DIR`加载指定路径下的配置文件
```sh
OPENCC_DATA_DIR=/path/to/your/config/dir opencc --help
```

### Experimental Plugins 試驗性插件

OpenCC 現已支援外部 C++ 分詞插件。當前第一個插件為 `opencc-jieba`，
可通過 `s2twp_jieba.json`、`tw2sp_jieba.json` 等插件配置啓用。

OpenCC now supports external C++ segmentation plugins. The first plugin is
`opencc-jieba`, which can be enabled through plugin-backed configs such as
`s2twp_jieba.json` and `tw2sp_jieba.json`.

注意：

- 該插件機制目前仍為試驗性功能。
- `jieba` 插件是可選組件，預設 OpenCC 構建、Python 套件和 Node.js 套件都不要求它。
- `opencc-jieba` 額外依賴 `cppjieba` 及其配套詞典資源，這些依賴僅在構建或分發該插件時需要。
- 在下一次正式發布版本之前，插件 ABI 仍可能發生變化，不應視為穩定介面。
- 我們預計從下一次正式發布版本開始，將插件 ABI 視為穩定介面。
- Windows 下插件必須與宿主 OpenCC 二進位使用 ABI 相容的工具鏈／執行時構建；MSVC 與 MinGW 產物不支援混用。

Notes:

- The plugin mechanism is currently experimental.
- The `jieba` plugin is optional and is not required for the default OpenCC
  build, Python package, or Node.js package.
- `opencc-jieba` additionally depends on `cppjieba` and its dictionary
  resources. These dependencies are only needed when building or distributing
  the plugin itself.
- The plugin ABI may still change before the next formal OpenCC release and
  should not yet be treated as stable.
- We expect to treat the plugin ABI as stable starting with the next formal
  OpenCC release.
- On Windows, plugins must be built with an ABI-compatible toolchain/runtime as
  the host OpenCC binary. Mixing MSVC-built hosts with MinGW-built plugins, or
  the reverse, is unsupported.

## Build 編譯

### Build with CMake

#### Linux & macOS

g++ 4.6+ or clang 3.2+ is required.

```bash
make
```

#### Windows Visual Studio:

```bash
build.cmd
```

### Build with Bazel

```bash
bazel build //:opencc
```

### Test 測試

#### Linux & macOS

```
make test
```

#### Windows Visual Studio:

```bash
test.cmd
```

#### Test with Bazel

```bash
bazel test --test_output=all //src/... //data/... //python/... //test/...
```

### Benchmark 基準測試

```
make benchmark
```

Example results (from Github CI, commit ID 9e80d5d, 2026-04-16, CMake macos-latest):

```
-------------------------------------------------------------------------
Benchmark                               Time             CPU   Iterations
-------------------------------------------------------------------------
BM_Initialization/hk2s                868 us          868 us          665
BM_Initialization/hk2t                139 us          139 us         5059
BM_Initialization/jp2t                203 us          203 us         3448
BM_Initialization/s2hk              26201 us        26200 us           27
BM_Initialization/s2t               26385 us        26382 us           27
BM_Initialization/s2tw              27108 us        27108 us           27
BM_Initialization/s2twp             26446 us        26445 us           25
BM_Initialization/s2twp_jieba      142754 us       141974 us            5
BM_Initialization/t2hk               66.7 us         66.7 us        10519
BM_Initialization/t2jp                166 us          166 us         4215
BM_Initialization/t2s                 797 us          797 us          883
BM_Initialization/t2tw               58.1 us         58.1 us        12075
BM_Initialization/tw2s                845 us          845 us          831
BM_Initialization/tw2sp              1004 us         1004 us          697
BM_Initialization/tw2t               93.3 us         93.3 us         7492
BM_ConvertLongText/s2t                327 ms          327 ms            2 bytes_per_second=5.45069M/s
BM_ConvertLongText/s2twp              554 ms          554 ms            1 bytes_per_second=3.21299M/s
BM_ConvertLongText/s2twp_jieba        742 ms          741 ms            1 bytes_per_second=2.40096M/s
BM_Convert/s2t_100                  0.649 ms        0.649 ms         1083 bytes_per_second=6.15628M/s
BM_Convert/s2t_1000                  6.64 ms         6.64 ms          106 bytes_per_second=6.16118M/s
BM_Convert/s2t_10000                 68.1 ms         68.1 ms           10 bytes_per_second=6.14608M/s
BM_Convert/s2t_100000                 718 ms          717 ms            1 bytes_per_second=5.96785M/s
BM_Convert/s2twp_100                 1.20 ms         1.20 ms          552 bytes_per_second=3.32407M/s
BM_Convert/s2twp_1000                12.3 ms         12.3 ms           57 bytes_per_second=3.32311M/s
BM_Convert/s2twp_10000                126 ms          126 ms            6 bytes_per_second=3.31205M/s
BM_Convert/s2twp_100000              1296 ms         1296 ms            1 bytes_per_second=3.3027M/s
BM_Convert/s2twp_jieba_100           1.51 ms         1.49 ms          495 bytes_per_second=2.67698M/s
BM_Convert/s2twp_jieba_1000          15.0 ms         15.0 ms           48 bytes_per_second=2.72292M/s
BM_Convert/s2twp_jieba_10000          153 ms          153 ms            5 bytes_per_second=2.73681M/s
BM_Convert/s2twp_jieba_100000        1728 ms         1728 ms            1 bytes_per_second=2.47784M/s
```

## Projects using OpenCC 使用 OpenCC 的項目

Please update if your project is using OpenCC.

* [ibus-pinyin](https://github.com/ibus/ibus-pinyin)
* [fcitx](https://github.com/fcitx/fcitx)
* [rimeime](https://rime.im/)
* [libgooglepinyin](http://code.google.com/p/libgooglepinyin/)
* [ibus-libpinyin](https://github.com/libpinyin/ibus-libpinyin)
* [alfred-chinese-converter](https://github.com/amowu/alfred-chinese-converter)
* [GoldenDict](https://github.com/goldendict/goldendict)
* [China Biographical Database Project (CBDB)](https://cbdb.hsites.harvard.edu/)

## License 許可協議

Apache License 2.0

## Third Party Library 第三方庫

* [darts-clone](https://github.com/s-yata/darts-clone) BSD License
* [marisa-trie](https://github.com/s-yata/marisa-trie) BSD License
* [tclap](http://tclap.sourceforge.net/) MIT License
* [rapidjson](https://github.com/Tencent/rapidjson) MIT License
* [Google Test](https://github.com/google/googletest) BSD License
* [cppjieba](https://github.com/yanyiwu/cppjieba) MIT License
  - Optional dependency used by the experimental `opencc-jieba` plugin.
  - 試驗性 `opencc-jieba` 插件使用的可選依賴。

## Change History 版本歷史

* [NEWS](https://github.com/BYVoid/OpenCC/blob/master/NEWS.md)

### Links 相關鏈接

* Introduction 詳細介紹 https://github.com/BYVoid/OpenCC/wiki/%E7%B7%A3%E7%94%B1
* 現代漢語常用簡繁一對多字義辨析表 http://ytenx.org/byohlyuk/KienxPyan

## Contributors 貢獻者

* [BYVoid](http://www.byvoid.com/)
* [佛振](https://github.com/lotem)
* [Peng Huang](https://github.com/phuang)
* [LI Daobing](https://github.com/lidaobing)
* [Kefu Chai](https://github.com/tchaikov)
* [Kan-Ru Chen](http://kanru.info/)
* [Ma Xiaojun](https://twitter.com/damage3025)
* [Jiang Jiang](http://jjgod.org/)
* [Ruey-Cheng Chen](https://github.com/rueycheng)
* [Paul Meng](http://home.mno2.org/)
* [Lawrence Lau](https://github.com/ktslwy)
* [瑾昀](https://github.com/kunki)
* [內木一郎](https://github.com/SyaoranHinata)
* [Marguerite Su](https://www.marguerite.su/)
* [Brian White](http://mscdex.net)
* [Qijiang Fan](https://fqj.me/)
* [LEOYoon-Tsaw](https://github.com/LEOYoon-Tsaw)
* [Steven Yao](https://github.com/stevenyao)
* [Pellaeon Lin](https://github.com/pellaeon)
* [stony](https://github.com/stony-shixz)
* [steelywing](https://github.com/steelywing)
* [吕旭东](https://github.com/lvxudong)
* [Weng Xuetian](https://github.com/wengxt)
* [Ma Tao](https://github.com/iwater)
* [Heinz Wiesinger](https://github.com/pprkut)
* [J.W](https://github.com/jakwings)
* [Amo Wu](https://github.com/amowu)
* [Mark Tsai](https://github.com/mxgit1090)
* [Zhe Wang](https://github.com/0x1997)
* [sgqy](https://github.com/sgqy)
* [Qichuan (Sean) ZHANG](https://github.com/qichuan)
* [Flandre Scarlet](https://github.com/XadillaX)
* [宋辰文](https://github.com/songchenwen)
* [iwater](https://github.com/iwater)
* [Xpol Wan](https://github.com/xpol)
* [Weihang Lo](https://github.com/weihanglo)
* [Cychih](https://github.com/pi314)
* [kyleskimo](https://github.com/kyleskimo)
* [Ryuan Choi](https://github.com/bunhere)
* [Prcuvu](https://github.com/Prcuvu)
* [Tony Able](https://github.com/TonyAble)
* [Xiao Liang](https://github.com/yxliang01)
* [Frank Lin](https://github.com/frankslin)

Please feel free to update this list if you have contributed OpenCC.
