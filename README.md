# Open Chinese Convert

開放中文轉換

An opensource project for conversion between Traditional Chinese and Simplified Chinese, supporting phrase-level conversion and regional idioms among Mainland China, Taiwan and Hong kong.

中文簡繁轉換開源項目，支持詞彙級别的轉換、異體字轉換和地區習慣用詞轉換（中國大陸、臺灣、香港）。

https://code.google.com/p/opencc/

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

### Build steps with CMake

1. Make a directory and check in.

    mkdir build
    cd build

2. Build sources.

    cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -D ENABLE_GETTEXT:BOOL=ON ..
    make

On windows, run these commands instead:

    cmake .. -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX="" -DCMAKE_BUILD_TYPE=Release -DENABLE_GETTEXT:BOOL=OFF
    make

3. Install.

    sudo make install

