import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import OpenCC from "../index.js";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

async function main() {
  const casesPath = path.join(__dirname, "testcases.json");
  const cases = JSON.parse(fs.readFileSync(casesPath, "utf-8"));
  if (!Array.isArray(cases) || cases.length === 0) {
    console.error("No testcases found");
    process.exit(1);
  }
  const byConfig = new Map();
  for (const tc of cases) {
    const cfg = tc.config;
    if (!byConfig.has(cfg)) {
      byConfig.set(cfg, OpenCC.Converter({ config: cfg }));
    }
  }

  let passed = 0;
  const failed = [];
  const results = [];
  for (const tc of cases) {
    const conv = byConfig.get(tc.config);
    const actual = await conv(tc.input);
    results.push({ ...tc, actual });
    if (actual === tc.expected) {
      passed++;
    } else {
      failed.push({ ...tc, actual });
    }
  }

  console.log(`Total: ${cases.length}, Passed: ${passed}, Failed: ${failed.length}`);
  if (failed.length) {
    failed.slice(0, 5).forEach((f, idx) => {
      console.log(`FAIL #${idx} [${f.config}]\n input: ${f.input}\n expect: ${f.expected}\n actual: ${f.actual}`);
    });
    process.exitCode = 1;
  }
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
