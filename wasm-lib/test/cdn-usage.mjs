#!/usr/bin/env node
/**
 * 测试直接导入 opencc-wasm.js 的 CDN 使用模式
 *
 * 模拟用法：
 * import initOpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/opencc-wasm.js";
 */

import { fileURLToPath } from "node:url";
import { readFileSync } from "node:fs";
import { dirname, join } from "node:path";

const __dirname = dirname(fileURLToPath(import.meta.url));
const rootDir = join(__dirname, "..");

console.log("🧪 测试 CDN 风格的使用方式\n");

try {
  // 1. 直接导入 WASM glue code（模拟从 CDN 加载）
  console.log("📦 步骤 1: 导入 opencc-wasm.js glue code...");
  const glueModulePath = join(rootDir, "dist/esm/opencc-wasm.js");
  const { default: initOpenCC } = await import(glueModulePath);
  console.log("✅ 成功导入\n");

  // 2. 初始化 WASM 模块
  console.log("🔧 步骤 2: 初始化 WASM 运行时...");
  const wasmModule = await initOpenCC();
  console.log("✅ WASM 运行时初始化成功\n");

  // 3. 包装 C API
  console.log("🔌 步骤 3: 包装 C API...");
  const api = {
    create: wasmModule.cwrap("opencc_create", "number", ["string"]),
    convert: wasmModule.cwrap("opencc_convert", "string", ["number", "string"]),
    destroy: wasmModule.cwrap("opencc_destroy", null, ["number"]),
  };
  console.log("✅ API 包装完成\n");

  // 4. 设置虚拟文件系统
  console.log("📁 步骤 4: 设置虚拟文件系统...");
  wasmModule.FS.mkdirTree("/data/config");
  wasmModule.FS.mkdirTree("/data/dict");
  console.log("✅ 文件系统准备完成\n");

  // 5. 加载配置
  console.log("📄 步骤 5: 加载 s2twp.json 配置...");
  const configPath = join(rootDir, "dist/data/config/s2twp.json");
  const configJson = JSON.parse(readFileSync(configPath, "utf-8"));

  // 6. 收集并加载字典
  console.log("📚 步骤 6: 收集并加载字典文件...");
  const dicts = new Set();
  function collectOcd2Files(node) {
    if (!node || typeof node !== "object") return;
    if (node.type === "ocd2" && node.file) dicts.add(node.file);
    if (node.type === "group" && Array.isArray(node.dicts)) {
      node.dicts.forEach(collectOcd2Files);
    }
  }
  collectOcd2Files(configJson.segmentation?.dict);
  if (Array.isArray(configJson.conversion_chain)) {
    configJson.conversion_chain.forEach(item => collectOcd2Files(item?.dict));
  }

  console.log(`   找到 ${dicts.size} 个字典文件`);
  for (const file of dicts) {
    const dictPath = join(rootDir, "dist/data/dict", file);
    const buf = readFileSync(dictPath);
    wasmModule.FS.writeFile(`/data/dict/${file}`, buf);
    console.log(`   ✓ ${file}`);
  }
  console.log("✅ 字典加载完成\n");

  // 7. 修改配置路径并写入
  console.log("⚙️  步骤 7: 修改配置路径...");
  function patchPaths(node) {
    if (!node || typeof node !== "object") return;
    if (node.type === "ocd2" && node.file) {
      node.file = `/data/dict/${node.file}`;
    }
    if (node.type === "group" && Array.isArray(node.dicts)) {
      node.dicts.forEach(patchPaths);
    }
  }
  patchPaths(configJson.segmentation?.dict);
  if (Array.isArray(configJson.conversion_chain)) {
    configJson.conversion_chain.forEach(item => patchPaths(item?.dict));
  }
  wasmModule.FS.writeFile("/data/config/s2twp.json", JSON.stringify(configJson));
  console.log("✅ 配置写入完成\n");

  // 8. 创建转换器
  console.log("🔨 步骤 8: 创建转换器实例...");
  const handle = api.create("/data/config/s2twp.json");
  if (!handle || handle < 0) {
    throw new Error("opencc_create failed");
  }
  console.log("✅ 转换器创建成功\n");

  // 9. 测试转换
  console.log("🧪 步骤 9: 测试转换功能...\n");

  const testCases = [
    "人人生而自由，在尊严和权利上一律平等",
    "鼠标里面的硅二极管坏了，导致光标分辨率降低。",
  ];

  console.log("简体 → 繁体转换测试：");
  console.log("─".repeat(60));

  for (const text of testCases) {
    const result = api.convert(handle, text);
    console.log(`输入: ${text}`);
    console.log(`输出: ${result}`);
    console.log("─".repeat(60));
  }

  // 10. 清理
  console.log("\n🧹 步骤 10: 清理资源...");
  api.destroy(handle);
  console.log("✅ 资源清理完成\n");

  console.log("🎉 所有测试通过！CDN 使用模式可以正常工作！\n");
  console.log("📝 实际使用示例：");
  console.log(`
  import initOpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/opencc-wasm.js";

  const wasmModule = await initOpenCC();
  const api = {
    create: wasmModule.cwrap("opencc_create", "number", ["string"]),
    convert: wasmModule.cwrap("opencc_convert", "string", ["number", "string"]),
  };

  // ... 加载配置和字典到 wasmModule.FS ...

  const handle = api.create("/data/config/s2twp.json");
  const result = api.convert(handle, "简体中文");
  `);

  process.exit(0);

} catch (err) {
  console.error("\n❌ 测试失败:", err.message);
  console.error(err.stack);
  process.exit(1);
}
