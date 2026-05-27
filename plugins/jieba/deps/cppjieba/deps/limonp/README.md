# limonp 

[![CMake](https://github.com/yanyiwu/limonp/actions/workflows/cmake.yml/badge.svg)](https://github.com/yanyiwu/limonp/actions/workflows/cmake.yml)
[![Author](https://img.shields.io/badge/author-@yanyiwu-blue.svg?style=flat)](http://yanyiwu.com/) 
[![Platform](https://img.shields.io/badge/platform-Linux,macOS,Windows-green.svg?style=flat)](https://github.com/yanyiwu/limonp)
[![Tag](https://img.shields.io/github/v/tag/yanyiwu/limonp.svg)](https://github.com/yanyiwu/limonp/releases)
[![License](https://img.shields.io/badge/license-MIT-yellow.svg?style=flat)](http://yanyiwu.mit-license.org)

## Introduction

`C++` headers(hpp) with Python style. 

## Feature

+ linking is a annoying thing, so I write these source code in headers file(`*.hpp`), you can use them only with `#include "xx.hpp"`, without linking *.a or *.so .

**`no linking , no hurts`** 

## Build

```
mkdir build
cmake ..
make

make test
```

## Example

See Details in `test/demo.cpp`

## Cases

1. [CppJieba]

## Contact

+ i@yanyiwu.com

[CppJieba]:https://github.com/yanyiwu/cppjieba.git
[muduo]:https://github.com/chenshuo/muduo.git

