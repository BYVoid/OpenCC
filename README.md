# Open Chinese Convert

## Introduction

Open Chinese Convert (OpenCC, 開放中文轉換) is an opensource project for conversion between Traditional Chinese and Simplified Chinese, supporting character-level conversion, phrase-level conversion, variant conversion and regional idioms among Mainland China, Taiwan and Hong kong.

中文簡繁轉換開源項目，支持詞彙級别的轉換、異體字轉換和地區習慣用詞轉換（中國大陸、臺灣、香港）。

### OpenCC特點

* 嚴格區分「一簡對多繁」和「一簡對多異」。
* 完全兼容異體字，可以實現動態替換。
* 嚴格審校一簡對多繁詞條，原則爲「能分則不合」。
* 支持中國大陸、臺灣、香港異體字和地區習慣用詞轉換，如「裏」「裡」、「鼠標」「滑鼠」。
* 使用歧義分割+最少分詞算法，儘可能從技術上優化轉換效果。
* 詞庫和函數庫完全分離，可以自由修改、導入、擴展。
* 支持C、C++、Python、PHP、Java、Ruby、Node.js。
* 兼容Windows、Linux、Mac平臺。
* 已經用於ibus-pinyin、fcitx的繁體模式輸入。

## Links

### Project home page
http://code.google.com/p/opencc/

### Introduction (詳細介紹)
https://code.google.com/p/opencc/wiki/Introduction

### Development Documentation
http://byvoid.github.io/OpenCC/

### Source Code on Github
https://github.com/byvoid/opencc

### OpenCC Online (在線轉換)
http://opencc.byvoid.com/

### 現代漢語常用簡繁一對多字義辨析表
http://ytenx.org/byohlyuk/KienxPyan

### Projects using Opencc

* [ibus-pinyin](http://code.google.com/p/ibus/)
* [fcitx](http://code.google.com/p/fcitx/)
* [rimeime](http://code.google.com/p/rimeime/)
* [libgooglepinyin](http://code.google.com/p/libgooglepinyin/)
* [ibus-libpinyin](https://github.com/libpinyin/ibus-libpinyin)
* [BYVBlog](https://github.com/byvoid/byvblog)
* [豆瓣同城微信](http://weixinqiao.com/douban-event/)

## Installation

### [Debian](http://packages.qa.debian.org/o/opencc.html)/[Ubuntu](https://launchpad.net/ubuntu/+source/opencc)

    apt-get install opencc

### [Fedora](https://admin.fedoraproject.org/pkgdb/acls/name/opencc)

    yum install opencc

### [Arch](https://www.archlinux.org/packages/community/x86_64/opencc/)

    pacman -S opencc

### [Mac](https://github.com/mxcl/homebrew/blob/master/Library/Formula/opencc.rb)

    brew install opencc

### [Node.js](https://npmjs.org/package/opencc)

    npm install opencc

## Usage

    $ opencc --help
    
    Open Chinese Convert (OpenCC) Command Line Tool

    Author: BYVoid <byvoid@byvoid.com>
    Bug Report: http://github.com/BYVoid/OpenCC/issues

    Usage:
     opencc [Options]

    Options:
     -i [file], --input=[file]   Read original text from [file].
     -o [file], --output=[file]  Write converted text to [file].
     -c [file], --config=[file]  Load configuration of conversion from [file].
     -v, --version               Print version and build information.
     -h, --help                  Print this help.

    With no input file, reads standard input and writes converted stream to standard output.
    Default configuration(zhs2zht.ini) will be loaded if not set.

## Build

[![Build Status](https://travis-ci.org/BYVoid/OpenCC.png?branch=master)](https://travis-ci.org/BYVoid/OpenCC)

### Build with CMake

Make a directory and check in:

    mkdir build
    cd build

Build sources:

    cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -D ENABLE_GETTEXT:BOOL=ON ..
    make

On windows, run these commands instead:

    cmake .. -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="" -DCMAKE_BUILD_TYPE=Release -DENABLE_GETTEXT:BOOL=OFF
    make

Install:

    sudo make install


### Build with gyp

    mkdir build
    gyp --depth . -D library=shared_library -f make --generator-output=build opencc.gyp
    make -C build

## Screenshot

![OpenCC Mac](http://opencc.googlecode.com/files/screenshot-gui-mac.png)

![OpenCC Windows](http://opencc.googlecode.com/files/screenshot-gui.png)

![OpenCC Ubuntu](http://opencc.googlecode.com/files/screenshot-gui-ubuntu.png)

## Contributors

* [BYVoid](http://www.byvoid.com/)
* [佛振](https://github.com/lotem)
* [Peng Huang](https://github.com/phuang)
* [LI Daobing](https://github.com/lidaobing)
