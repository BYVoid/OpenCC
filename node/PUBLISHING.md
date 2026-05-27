# Publishing Guide

This document describes how to prepare and publish the OpenCC Node.js bindings.

## Prerequisites
- **Platform Assumption**: This guide assumes the publishing process is executed on an **ARM64 Mac** computer. `darwin-arm64` generation uses local compilation, while Linux `x64` / `arm64` generation uses BuildBuddy remote runners.
- Make sure you have BuildBuddy (or another Bazel remote execution environment) configured in your local `~/.bazelrc` for cross-compiling Linux binaries.

## Publishing Steps

1. **Install Dependencies**
   ```bash
   npm install
   ```

2. **Generate Prebuilds**
   Run the following command on macOS to generate all cross-platform native addons:
   ```bash
   npm run prebuild:all
   ```
   This script will automatically:
   - Build macOS native addons locally using `prebuildify`. (Note: This only generates the binary for the host's current architecture, e.g., `darwin-arm64` on Apple Silicon. Currently, no Windows prebuilds are generated).
   - Trigger the Bazel build script (`scripts/build-node-prebuild-bazel.sh`) to remotely cross-compile Linux native addons (`linux-x64` and `linux-arm64`) with `-c opt` for optimized, stripped binaries.
   - Collect all generated `.node` files into the `prebuilds/` directory.

3. **Run Tests**
   Verify that the bindings work correctly:
   ```bash
   npm test
   ```

4. **Publish**
   Finally, publish the package to npm:
   ```bash
   npm publish
   ```

---

# 發佈指南

這份文件說明了如何準備與發佈 OpenCC 的 Node.js 綁定。

## 前置作業
- **平臺假設**：本指南假定發佈過程在一個 **ARM64 Mac 计算机**上執行。`darwin-arm64` 生成使用本機編譯；Linux `x64` / `arm64` 使用 BuildBuddy 遠程 runner 生成。
- 請確保您的本地環境（如 `~/.bazelrc`）已設定好 BuildBuddy 或其他的 Bazel 遠端執行環境，以便跨平臺編譯 Linux 二進位檔案。

## 發佈步驟

1. **安裝依賴**
   ```bash
   npm install
   ```

2. **生成預編譯二進位檔 (Prebuilds)**
   請在 macOS 上執行以下指令，以生成所有跨平臺的原生模組：
   ```bash
   npm run prebuild:all
   ```
   這個指令會自動執行以下操作：
   - 使用 `prebuildify` 在本地編譯 macOS 的原生模組。（註：此步驟僅會生成與當前主機相同架構的二進位檔，例如 Apple Silicon 機器只會生成 `darwin-arm64`。目前尚未提供 Windows 平台的 prebuilds）。
   - 觸發 Bazel 腳本（`scripts/build-node-prebuild-bazel.sh`），透過遠端編譯產生經過優化與剔除除錯符號（`-c opt`）的 Linux 原生模組（包含 `linux-x64` 與 `linux-arm64`）。
   - 將所有編譯好的 `.node` 檔案統一收集至 `prebuilds/` 目錄中。

3. **執行測試**
   確認模組運作正常：
   ```bash
   npm test
   ```

4. **發佈至 npm**
   最後，將套件發佈至 npm 暫存庫：
   ```bash
   npm publish
   ```
