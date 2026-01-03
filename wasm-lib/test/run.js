import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import OpenCC from "../index.js";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

async function main() {
  const casesPath = path.join(__dirname, "testcases.json");
  const parsed = JSON.parse(fs.readFileSync(casesPath, "utf-8"));
  const cases = parsed?.cases || [];
  if (cases.length === 0) {
    console.error("No testcases found");
    process.exit(1);
  }
  const converters = new Map();

  let passed = 0;
  const failed = [];
  const results = [];
  for (const tc of cases) {
    if (!tc.expected || typeof tc.expected !== "object") continue;
    for (const [cfg, expected] of Object.entries(tc.expected)) {
      const configName = `${cfg}.json`;
      if (!converters.has(configName)) {
        converters.set(configName, OpenCC.Converter({ config: configName }));
      }
      const conv = converters.get(configName);
      const actual = await conv(tc.input);
      results.push({ ...tc, config: configName, actual });
      if (actual === expected) {
        passed++;
      } else {
        failed.push({ ...tc, config: configName, expected, actual });
      }
    }
  }

  console.log(`Total: ${results.length}, Passed: ${passed}, Failed: ${failed.length}`);
  if (failed.length) {
    failed.slice(0, 5).forEach((f, idx) => {
      console.log(
        `FAIL #${idx} [${f.config}]\n input: ${f.input}\n expect: ${f.expected}\n actual: ${f.actual}`
      );
    });
    process.exitCode = 1;
  }
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
