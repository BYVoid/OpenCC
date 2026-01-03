import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { test } from "node:test";
import OpenCC from "../index.js";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const casesPath = path.join(__dirname, "testcases.json");
const parsed = JSON.parse(fs.readFileSync(casesPath, "utf-8"));
const cases = parsed?.cases || [];

const converterCache = new Map();
function getConverter(config) {
  if (!converterCache.has(config)) {
    converterCache.set(config, OpenCC.Converter({ config }));
  }
  return converterCache.get(config);
}

cases.forEach((tc, idx) => {
  if (!tc.expected || typeof tc.expected !== "object") return;
  Object.entries(tc.expected).forEach(([cfg, expected]) => {
    const configName = `${cfg}.json`;
    test(`[${configName}] case #${idx + 1}${tc.id ? ` (${tc.id})` : ""}`, async () => {
      const convert = getConverter(configName);
      const actual = await convert(tc.input);
      assert.strictEqual(actual, expected);
    });
  });
});
