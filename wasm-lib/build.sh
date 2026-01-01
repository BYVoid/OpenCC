#!/usr/bin/env bash
set -euo pipefail

# 输出目录（中间构建产物）
BUILD_DIR="build"
mkdir -p "${BUILD_DIR}"

# 单独的 emcc 缓存目录，避免权限问题
export EM_CACHE="$(pwd)/.emcache"
mkdir -p "${EM_CACHE}"

# OpenCC 源码路径（相对仓库根）
OPENCC_SRC_DIR=".."

# 最小依赖的 OpenCC 源文件
OPENCC_SRCS=(
  ${OPENCC_SRC_DIR}/src/BinaryDict.cpp
  ${OPENCC_SRC_DIR}/src/Config.cpp
  ${OPENCC_SRC_DIR}/src/Conversion.cpp
  ${OPENCC_SRC_DIR}/src/ConversionChain.cpp
  ${OPENCC_SRC_DIR}/src/Converter.cpp
  ${OPENCC_SRC_DIR}/src/Dict.cpp
  ${OPENCC_SRC_DIR}/src/DictEntry.cpp
  ${OPENCC_SRC_DIR}/src/DictGroup.cpp
  ${OPENCC_SRC_DIR}/src/Lexicon.cpp
  ${OPENCC_SRC_DIR}/src/MarisaDict.cpp
  ${OPENCC_SRC_DIR}/src/MaxMatchSegmentation.cpp
  ${OPENCC_SRC_DIR}/src/SerializedValues.cpp
  ${OPENCC_SRC_DIR}/src/SimpleConverter.cpp
  ${OPENCC_SRC_DIR}/src/TextDict.cpp
  ${OPENCC_SRC_DIR}/src/UTF8StringSlice.cpp
  ${OPENCC_SRC_DIR}/src/UTF8Util.cpp
)

# marisa trie 源文件
MARISA_DIR="${OPENCC_SRC_DIR}/deps/marisa-0.2.6"
MARISA_SRCS=(
  ${MARISA_DIR}/lib/marisa/agent.cc
  ${MARISA_DIR}/lib/marisa/keyset.cc
  ${MARISA_DIR}/lib/marisa/trie.cc
  ${MARISA_DIR}/lib/marisa/grimoire/io/reader.cc
  ${MARISA_DIR}/lib/marisa/grimoire/io/writer.cc
  ${MARISA_DIR}/lib/marisa/grimoire/io/mapper.cc
  ${MARISA_DIR}/lib/marisa/grimoire/trie/louds-trie.cc
  ${MARISA_DIR}/lib/marisa/grimoire/trie/tail.cc
  ${MARISA_DIR}/lib/marisa/grimoire/vector/bit-vector.cc
)

# 头文件搜索路径
INCLUDE_FLAGS=(
  -I${OPENCC_SRC_DIR}/src
  -I${MARISA_DIR}/include
  -I${MARISA_DIR}/lib
  -I${OPENCC_SRC_DIR}/deps/rapidjson-1.1.0
)

# 编译选项：
# -DOPENCC_WASM_WITH_OPENCC: 启用真实 OpenCC 逻辑
# -s MODULARIZE=1: 生成可按需加载的模块工厂函数
# -s EXPORT_NAME: 自定义工厂函数名，便于前端调用
# -s EXPORTED_FUNCTIONS: 导出 C 接口（需要前缀下划线）
# -s EXPORTED_RUNTIME_METHODS: 暴露 cwrap/FS/ccall，便于 JS 侧调用
# -O2: 体积/性能权衡
COMMON_FLAGS=(
  -DOPENCC_WASM_WITH_OPENCC
  "${OPENCC_SRCS[@]}"
  "${MARISA_SRCS[@]}"
  src/main.cpp
  "${INCLUDE_FLAGS[@]}"
  -fexceptions
  -sDISABLE_EXCEPTION_CATCHING=0
  -O2
  -s WASM=1
  -s MODULARIZE=1
  -s FORCE_FILESYSTEM=1
  -s ALLOW_MEMORY_GROWTH=1
  -s EXPORT_NAME="createOpenCCWasm"
  -s EXPORTED_FUNCTIONS="['_opencc_create','_opencc_convert','_opencc_destroy']"
  -s EXPORTED_RUNTIME_METHODS="['cwrap','FS','ccall']"
)

# ES module（适合浏览器 / 现代 bundler）
em++ \
  "${COMMON_FLAGS[@]}" \
  -s EXPORT_ES6=1 \
  -o "${BUILD_DIR}/opencc-wasm.esm.js"

# CommonJS（适合 Node.js require）
em++ \
  "${COMMON_FLAGS[@]}" \
  -s EXPORT_ES6=0 \
  -s ENVIRONMENT='node' \
  -o "${BUILD_DIR}/opencc-wasm.cjs"

# WASM 文件由 emcc 自动生成
echo "Build complete. Intermediate files in ${BUILD_DIR}/"
echo "  - ${BUILD_DIR}/opencc-wasm.esm.js (ESM glue for tests/rebuild)"
echo "  - ${BUILD_DIR}/opencc-wasm.cjs (CJS glue for tests/rebuild)"
echo "  - ${BUILD_DIR}/opencc-wasm.wasm (WASM binary)"
echo ""
echo "Run 'node scripts/build-api.js' to generate dist/ for publishing."
