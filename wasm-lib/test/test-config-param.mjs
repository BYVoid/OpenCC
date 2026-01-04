#!/usr/bin/env node
/**
 * 测试 config 参数 - 直接指定配置名称
 *
 * 新用法：OpenCC.Converter({ config: "s2twp" })
 * 替代：OpenCC.Converter({ from: "cn", to: "twp" })
 */

import { join, dirname } from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = dirname(fileURLToPath(import.meta.url));
const distPath = join(__dirname, "../dist/esm/index.js");

console.log("🧪 测试 config 参数（直接指定配置名称）\n");

try {
  const { default: OpenCC } = await import(distPath);

  // 测试用例
  const testCases = [
    {
      name: "s2t (简体→繁体)",
      config: "s2t",
      input: "简体中文转换",
      expected: "簡體中文轉換"
    },
    {
      name: "s2tw (简体→台湾)",
      config: "s2tw",
      input: "鼠标和软件",
      expected: "滑鼠和軟體"
    },
    {
      name: "s2twp (简体→台湾惯用词)",
      config: "s2twp",
      input: "服务器上的文件",
      expected: "伺服器上的檔案"
    },
    {
      name: "s2hk (简体→香港)",
      config: "s2hk",
      input: "打印机",
      expected: "打印機"
    },
    {
      name: "t2s (繁体→简体)",
      config: "t2s",
      input: "繁體中文",
      expected: "繁体中文"
    },
    {
      name: "tw2s (台湾→简体)",
      config: "tw2s",
      input: "滑鼠軟體",
      expected: "鼠标软件"
    },
    {
      name: "hk2s (香港→简体)",
      config: "hk2s",
      input: "打印機",
      expected: "打印机"
    },
  ];

  console.log("方式 1: 使用 config 参数（新方式）");
  console.log("=".repeat(70));

  for (const testCase of testCases) {
    console.log(`\n📝 ${testCase.name}`);
    console.log(`   配置: config: "${testCase.config}"`);

    // 使用 config 参数
    const converter1 = OpenCC.Converter({ config: testCase.config });
    const result1 = await converter1(testCase.input);

    console.log(`   输入: ${testCase.input}`);
    console.log(`   输出: ${result1}`);

    // 验证
    if (result1 !== testCase.expected) {
      console.log(`   ⚠️  期望: ${testCase.expected}`);
    } else {
      console.log(`   ✅ 正确`);
    }
  }

  console.log("\n" + "=".repeat(70));
  console.log("\n方式 2: 对比 from/to 参数（传统方式）");
  console.log("=".repeat(70));

  // 对比测试
  const compareTests = [
    {
      name: "简体→台湾惯用词",
      config: "s2twp",
      from: "cn",
      to: "twp",
      input: "服务器软件"
    },
    {
      name: "台湾→简体",
      config: "tw2s",
      from: "tw",
      to: "cn",
      input: "滑鼠軟體"
    }
  ];

  for (const test of compareTests) {
    console.log(`\n📝 ${test.name}`);

    // 方式 1: config 参数
    const converter1 = OpenCC.Converter({ config: test.config });
    const result1 = await converter1(test.input);

    // 方式 2: from/to 参数
    const converter2 = OpenCC.Converter({ from: test.from, to: test.to });
    const result2 = await converter2(test.input);

    console.log(`   输入: ${test.input}`);
    console.log(`   config: "${test.config}" → ${result1}`);
    console.log(`   from/to: "${test.from}"→"${test.to}" → ${result2}`);

    if (result1 === result2) {
      console.log(`   ✅ 两种方式结果一致`);
    } else {
      console.log(`   ❌ 结果不一致！`);
    }
  }

  console.log("\n" + "=".repeat(70));
  console.log("\n支持的配置文件（直接使用 config 参数）:");
  console.log("=".repeat(70));

  const allConfigs = [
    "s2t      - 简体 → 繁体",
    "s2tw     - 简体 → 台湾",
    "s2twp    - 简体 → 台湾（惯用词）",
    "s2hk     - 简体 → 香港",
    "t2s      - 繁体 → 简体",
    "t2tw     - 繁体 → 台湾",
    "t2hk     - 繁体 → 香港",
    "t2jp     - 繁体 → 日文新字体",
    "tw2s     - 台湾 → 简体",
    "tw2sp    - 台湾 → 简体（惯用词）",
    "tw2t     - 台湾 → 繁体",
    "hk2s     - 香港 → 简体",
    "hk2t     - 香港 → 繁体",
    "jp2t     - 日文新字体 → 繁体",
  ];

  allConfigs.forEach(config => {
    console.log(`  • ${config}`);
  });

  console.log("\n用法示例：");
  console.log(`  const converter = OpenCC.Converter({ config: "s2twp" });`);
  console.log(`  const result = await converter("服务器");  // 伺服器`);

  console.log("\n✅ 所有测试完成！\n");

  process.exit(0);

} catch (err) {
  console.error("\n❌ 测试失败:", err.message);
  console.error(err.stack);
  process.exit(1);
}
