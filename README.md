# Open Chinese Convert 開放中文轉換

[![Travis](https://img.shields.io/travis/BYVoid/OpenCC.svg)](https://travis-ci.org/BYVoid/OpenCC)
[![AppVeyor](https://img.shields.io/appveyor/ci/Carbo/OpenCC.svg)](https://ci.appveyor.com/project/Carbo/OpenCC)
[![Python package](https://github.com/BYVoid/OpenCC/workflows/Python%20package/badge.svg?branch=master)](https://github.com/BYVoid/OpenCC/actions?query=workflow%3A%22Python+package%22)

## Introduction 介紹

![OpenCC](https://opencc.byvoid.com/img/opencc.png)

Open Chinese Convert (OpenCC, 開放中文轉換) is an opensource project for conversions between Traditional Chinese, Simplified Chinese and Japanese Kanji (Shinjitai). It supports character-level and phrase-level conversion, character variant conversion and regional idioms among Mainland China, Taiwan and Hong Kong. This is not translation tool between Mandarin and Cantonese, etc.

中文簡繁轉換開源項目，支持詞彙級別的轉換、異體字轉換和地區習慣用詞轉換（中國大陸、臺灣、香港、日本新字體）。不提供普通話與粵語的轉換。

Discussion (Telegram): https://t.me/open_chinese_convert

### Features 特點

* 嚴格區分「一簡對多繁」和「一簡對多異」。
* 完全兼容異體字，可以實現動態替換。
* 嚴格審校一簡對多繁詞條，原則爲「能分則不合」。
* 支持中國大陸、臺灣、香港異體字和地區習慣用詞轉換，如「裏」「裡」、「鼠標」「滑鼠」。
* 詞庫和函數庫完全分離，可以自由修改、導入、擴展。

## Installation 安裝

See [Download](https://github.com/BYVoid/OpenCC/wiki/Download).

## Usage 使用

### Online demo 線上轉換展示

Warning: **This is NOT an API.** You will be banned if you make calls programmatically.

https://opencc.byvoid.com/

### Node.js

[npm](https://www.npmjs.com/opencc) `npm i install opencc`

#### JavaScript
```js
const OpenCC = require('opencc');
const converter = new OpenCC('s2t.json');
converter.convertPromise("汉字").then(converted => {
  console.log(converted);  // 漢字
});
```

#### TypeScript
```ts
import { OpenCC } from 'opencc';
async function main() {
  const converter: OpenCC = new OpenCC('s2t.json');
  const result: string = await converter.convertPromise('汉字');
  console.log(result);
}
```

See [demo.js](https://github.com/BYVoid/OpenCC/blob/master/node/demo.js) and [ts-demo.ts](https://github.com/BYVoid/OpenCC/blob/master/node/ts-demo.ts).

### Python

[PyPI](https://pypi.org/project/OpenCC/) `pip install opencc` (Windows, Linux, Mac)

```python
import opencc
converter = opencc.OpenCC('s2t.json')
converter.convert('汉字')  # 漢字
```

### C++

```c++
#include "opencc.h"

int main() {
  const SimpleConverter converter("s2t.json");
  converter.Convert("汉字");  // 漢字
  return 0;
}
```

Document 文檔: https://byvoid.github.io/OpenCC/

### Command Line

* `opencc --help`
* `opencc_dict --help`
* `opencc_phrase_extract --help`

### Others (Unofficial)

* Swift (iOS): [SwiftyOpenCC](https://github.com/XQS6LB3A/SwiftyOpenCC)
* Java: [opencc4j](https://github.com/houbb/opencc4j)
* Android: [android-opencc](https://github.com/qichuan/android-opencc)
* PHP: [opencc4php](https://github.com/nauxliu/opencc4php)
* WebAssembly: [wasm-opencc](https://github.com/oyyd/wasm-opencc)

### Configurations 配置文件

#### 預設配置文件

* `s2t.json` Simplified Chinese to Traditional Chinese 簡體到繁體
* `t2s.json` Traditional Chinese to Simplified Chinese 繁體到簡體
* `s2tw.json` Simplified Chinese to Traditional Chinese (Taiwan Standard) 簡體到臺灣正體
* `tw2s.json` Traditional Chinese (Taiwan Standard) to Simplified Chinese 臺灣正體到簡體
* `s2hk.json` Simplified Chinese to Traditional Chinese (Hong Kong variant) 簡體到香港繁體
* `hk2s.json` Traditional Chinese (Hong Kong variant) to Simplified Chinese 香港繁體到簡體
* `s2twp.json` Simplified Chinese to Traditional Chinese (Taiwan Standard) with Taiwanese idiom 簡體到繁體（臺灣正體標準）並轉換爲臺灣常用詞彙
* `tw2sp.json` Traditional Chinese (Taiwan Standard) to Simplified Chinese with Mainland Chinese idiom 繁體（臺灣正體標準）到簡體並轉換爲中國大陸常用詞彙
* `t2tw.json` Traditional Chinese (OpenCC Standard) to Taiwan Standard 繁體（OpenCC 標準）到臺灣正體
* `hk2t.json` Traditional Chinese (Hong Kong variant) to Traditional Chinese 香港繁體到繁體（OpenCC 標準）
* `t2hk.json` Traditional Chinese (OpenCC Standard) to Hong Kong variant 繁體（OpenCC 標準）到香港繁體
* `t2jp.json` Traditional Chinese Characters (Kyūjitai) to New Japanese Kanji (Shinjitai) 繁體（OpenCC 標準，舊字體）到日文新字體
* `jp2t.json` New Japanese Kanji (Shinjitai) to Traditional Chinese Characters (Kyūjitai) 日文新字體到繁體（OpenCC 標準，舊字體）
* `tw2t.json` Traditional Chinese (Taiwan standard) to Traditional Chinese 臺灣正體到繁體（OpenCC 標準）

## Build 編譯

### Build with CMake

#### Linux & Mac OS X

g++ 4.6+ or clang 3.2+ is required.

```bash
make
```

#### Windows Visual Studio:

```bash
build.cmd
```

### Test 測試

#### Linux & Mac OS X

```
make test
```

#### Windows Visual Studio:

```bash
test.cmd
```

### Benchmark 基準測試

```
make benchmark
```

Example results (from Travis CI):

```
1: ------------------------------------------------------------------
1: Benchmark                        Time             CPU   Iterations
1: ------------------------------------------------------------------
1: BM_Initialization/hk2s     1587215 ns      1587485 ns          435
1: BM_Initialization/hk2t      126112 ns       125976 ns         5384
1: BM_Initialization/jp2t      245646 ns       245414 ns         2847
1: BM_Initialization/s2hk    26017749 ns     26017390 ns           27
1: BM_Initialization/s2t     26298084 ns     26296375 ns           27
1: BM_Initialization/s2tw    26483120 ns     26482164 ns           27
1: BM_Initialization/s2twp   26455564 ns     26454666 ns           26
1: BM_Initialization/t2hk       44759 ns        44636 ns        15733
1: BM_Initialization/t2jp      143401 ns       143227 ns         4876
1: BM_Initialization/t2s      1374298 ns      1373979 ns          510
1: BM_Initialization/tw2s     1443389 ns      1443701 ns          464
1: BM_Initialization/tw2sp    1699645 ns      1699823 ns          399
1: BM_Initialization/tw2t       76294 ns        76083 ns         9229
1: BM_Convert                     581 ms          581 ms            1
1/1 Test #1: BenchmarkTest ....................   Passed   14.49 sec
```

## Projects using OpenCC 使用 OpenCC 的項目

* [ibus-pinyin](https://github.com/ibus/ibus-pinyin)
* [fcitx](https://github.com/fcitx/fcitx)
* [rimeime](https://rime.im/)
* [libgooglepinyin](http://code.google.com/p/libgooglepinyin/)
* [ibus-libpinyin](https://github.com/libpinyin/ibus-libpinyin)
* [alfred-chinese-converter](https://github.com/amowu/alfred-chinese-converter)
* [GoldenDict](https://github.com/goldendict/goldendict)

## License 許可協議

Apache License 2.0

## Third Party Library 第三方庫

* [darts-clone](https://github.com/s-yata/darts-clone) BSD License
* [marisa-trie](https://github.com/s-yata/marisa-trie) BSD License
* [tclap](http://tclap.sourceforge.net/) MIT License
* [rapidjson](https://github.com/Tencent/rapidjson) MIT License
* [Google Test](https://github.com/google/googletest) BSD License

All these libraries are statically linked.

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
* [Tony Able](https://github.com/TonyAble)
* [Xiao Liang](https://github.com/yxliang01)

Please update this list you have contributed OpenCC.
