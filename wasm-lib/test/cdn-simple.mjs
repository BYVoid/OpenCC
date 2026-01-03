#!/usr/bin/env node
/**
 * 简化版 CDN 使用示例 - 使用高级 API
 *
 * 实际使用：
 * import OpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/index.js";
 */

import { join, dirname } from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = dirname(fileURLToPath(import.meta.url));
const distPath = join(__dirname, "../dist/esm/index.js");

console.log("🎯 测试高级 API（简化使用）\n");

try {
  // 导入高级 API
  const { default: OpenCC } = await import(distPath);

  // 创建转换器（简体 → 繁体）
  console.log("📝 创建简体转繁体转换器...");
  const converter = OpenCC.Converter({ from: "cn", to: "t" });

  // 测试转换
  const testTexts = [
    "人人生而自由，在尊严和权利上一律平等",
    "鼠标里面的硅二极管坏了，导致光标分辨率降低。",
  ];

  console.log("\n简体 → 繁体转换结果：");
  console.log("=".repeat(60));

  for (const text of testTexts) {
    const result = await converter(text);
    console.log(`输入: ${text}`);
    console.log(`输出: ${result}`);
    console.log("-".repeat(60));
  }

  console.log("\n✅ 测试完成！\n");
  console.log("📝 实际使用示例：");
  console.log(`
  // 从 CDN 导入
  import OpenCC from "https://cdn.jsdelivr.net/npm/opencc-wasm@0.2.1/dist/esm/index.js";

  // 创建转换器
  const converter = OpenCC.Converter({ from: "cn", to: "t" });

  // 转换文本
  const result = await converter("简体中文");
  console.log(result);  // 輸出: 簡體中文
  `);

  process.exit(0);

} catch (err) {
  console.error("\n❌ 测试失败:", err.message);
  console.error(err.stack);
  process.exit(1);
}
