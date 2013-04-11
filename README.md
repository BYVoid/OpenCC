# Open Chinese Convert

## Introduction

Open Chinese Convert (OpenCC, 開放中文轉換) is an opensource project for conversion between Traditional Chinese and Simplified Chinese, supporting character-level conversion, phrase-level conversion, variant conversion and regional idioms among Mainland China, Taiwan and Hong kong.

中文簡繁轉換開源項目，支持詞彙級别的轉換、異體字轉換和地區習慣用詞轉換（中國大陸、臺灣、香港）。

## Documentations

Project home page https://code.google.com/p/opencc/

Introduction (詳細介紹) https://code.google.com/p/opencc/wiki/Introduction

Development Documentation http://byvoid.github.io/OpenCC/

## Projects using Opencc

* [ibus-pinyin](http://code.google.com/p/ibus/)
* [fcitx](http://code.google.com/p/fcitx/)
* [rimeime](http://code.google.com/p/rimeime/)
* [libgooglepinyin](http://code.google.com/p/libgooglepinyin/)
* [ibus-libpinyin](https://github.com/libpinyin/ibus-libpinyin)

## Installation

### Debian/Ubuntu

    apt-get install opencc

### Fedora

    yum install opencc

### Arch

    yaourt -S opencc

### Mac OS

    brew install opencc

### Node.js

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

## Screenshoots

![OpenCC Mac](http://opencc.googlecode.com/files/screenshot-gui-mac.png)

![OpenCC Windows](http://opencc.googlecode.com/files/screenshot-gui.png)

![OpenCC Ubuntu](http://opencc.googlecode.com/files/screenshot-gui-ubuntu.png)
