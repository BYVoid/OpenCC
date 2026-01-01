import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { test } from "node:test";
import OpenCC from "../index.js";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const casesPath = path.join(__dirname, "testcases.json");
const cases = JSON.parse(fs.readFileSync(casesPath, "utf-8"));

const converterCache = new Map();
function getConverter(config) {
  if (!converterCache.has(config)) {
    converterCache.set(config, OpenCC.Converter({ config }));
  }
  return converterCache.get(config);
}

cases.forEach((tc, idx) => {
  test(`[${tc.config}] case #${idx + 1}`, async () => {
    const convert = getConverter(tc.config);
    const actual = await convert(tc.input);
    assert.strictEqual(actual, tc.expected);
  });
});
