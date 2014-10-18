# Open Chinese Convert 開放中文轉換

## Introduction 介紹

Open Chinese Convert (OpenCC, 開放中文轉換) is an opensource project for conversion between Traditional Chinese and Simplified Chinese, supporting character-level conversion, phrase-level conversion, variant conversion and regional idioms among Mainland China, Taiwan and Hong kong.

中文簡繁轉換開源項目，支持詞彙級別的轉換、異體字轉換和地區習慣用詞轉換（中國大陸、臺灣、香港）。

### Features 特點

* 嚴格區分「一簡對多繁」和「一簡對多異」。
* 完全兼容異體字，可以實現動態替換。
* 嚴格審校一簡對多繁詞條，原則爲「能分則不合」。
* 支持中國大陸、臺灣、香港異體字和地區習慣用詞轉換，如「裏」「裡」、「鼠標」「滑鼠」。
* 詞庫和函數庫完全分離，可以自由修改、導入、擴展。
* 支持C、C++、Python、PHP、Java、Ruby、Node.js。
* 兼容Windows、Linux、Mac平臺。

### Links 相關鏈接

* Introduction 詳細介紹 https://github.com/BYVoid/OpenCC/wiki/%E7%B7%A3%E7%94%B1
* OpenCC Online (在線轉換) http://opencc.byvoid.com/
* 現代漢語常用簡繁一對多字義辨析表 http://ytenx.org/byohlyuk/KienxPyan

## Installation 安裝

* [Debian](http://packages.qa.debian.org/o/opencc.html)
* [Ubuntu](https://launchpad.net/ubuntu/+source/opencc)
* [Fedora](https://admin.fedoraproject.org/pkgdb/package/opencc/)
* [Arch Linux](https://www.archlinux.org/packages/community/x86_64/opencc/)
* [Mac OS](https://github.com/mxcl/homebrew/blob/master/Library/Formula/opencc.rb)
* [Node.js](https://npmjs.org/package/opencc)

## Download 下載

https://bintray.com/byvoid/opencc/OpenCC

## Usage 使用

### Command Line 命令行

`opencc --help`

### Configurations 配置文件

#### 預設配置文件

* `s2t.json` Simplified Chinese to Traditional Chinese 簡體到繁體
* `t2s.json` Traditional Chinese to Simplified Chinese 繁體到簡體
* `s2tw.json` Simplified Chinese to Traditional Chinese (Taiwan Standard) 簡體到臺灣正體
* `tw2s.json` Traditional Chinese (Taiwan Standard) to Simplified Chinese 臺灣正體到簡體
* `s2hk.json` Simplified Chinese to Traditional Chinese (Hong Kong Standard) 簡體到香港繁體（香港小學學習字詞表標準）
* `hk2s.json` Traditional Chinese (Hong Kong Standard) to Simplified Chinese 香港繁體（香港小學學習字詞表標準）到簡體
* `s2twp.json` Simplified Chinese to Traditional Chinese (Taiwan Standard) with Taiwanese idiom 簡體到繁體（臺灣正體標準）並轉換爲臺灣常用詞彙
* `tw2sp.json` Traditional Chinese (Taiwan Standard) to Simplified Chinese with Mainland Chinese idiom 繁體（臺灣正體標準）到簡體並轉換爲中國大陸常用詞彙

## Development Documentation 開發文檔

* http://byvoid.github.io/OpenCC/

## Build 編譯

[![Build Status](https://travis-ci.org/BYVoid/OpenCC.png?branch=master)](https://travis-ci.org/BYVoid/OpenCC)

### Build with CMake

Linux/OSX (gcc 4.6 or clang 3.2 is required):

```
make
sudo make install
```

Windows MSYS:

```
cmake .. -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="" -DCMAKE_BUILD_TYPE=Release
make
```

Windows Visual Studio (2013 or higher required):

```
cmake .. -G "Visual Studio 12" -DCMAKE_INSTALL_PREFIX="" -DCMAKE_BUILD_TYPE=Release
make
```

### Projects using Opencc 使用OpenCC的項目

* [ibus-pinyin](http://code.google.com/p/ibus/)
* [fcitx](http://code.google.com/p/fcitx/)
* [rimeime](http://code.google.com/p/rimeime/)
* [libgooglepinyin](http://code.google.com/p/libgooglepinyin/)
* [ibus-libpinyin](https://github.com/libpinyin/ibus-libpinyin)
* [BYVBlog](https://github.com/byvoid/byvblog)
* [豆瓣同城微信](http://weixinqiao.com/douban-event/)

## License 許可協議

Apache License 2.0

## Third Party Library 第三方庫

* [darts-clone](https://code.google.com/p/darts-clone/) BSD License
* [tclap](http://tclap.sourceforge.net/) MIT License
* [rapidjson](https://github.com/miloyip/rapidjson) MIT License

All these libraries are statically linked.

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
* [Marguerite Su](https://www.marguerite.su/)
* [Brian White](http://mscdex.net)
* [Qijiang Fan](https://fqj.me/)
* [LEOYoon-Tsaw](https://github.com/LEOYoon-Tsaw)
* [Steven Yao](https://github.com/stevenyao)
* [Pellaeon Lin](https://github.com/pellaeon)
* [stony](https://github.com/stony-shixz)
* [steelywing](https://github.com/steelywing)
* [吕旭东](https://github.com/lvxudong)
