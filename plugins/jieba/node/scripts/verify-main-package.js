#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const packageJson = require(path.join(packageRoot, 'package.json'));

const requiredFiles = [
  'index.js',
  'README.md',
  'package.json',
  'data/s2hk_jieba.json',
  'data/s2t_jieba.json',
  'data/s2tw_jieba.json',
  'data/s2twp_jieba.json',
  'data/tw2sp_jieba.json',
  'data/jieba_dict/jieba_merged.ocd2',
  'data/jieba_dict/hmm_model.utf8',
  'data/jieba_dict/idf.utf8',
  'data/jieba_dict/stop_words.utf8',
];

const expectedOptionalPackages = [
  '@opencc/opencc-jieba-darwin-arm64',
  '@opencc/opencc-jieba-linux-arm64',
  '@opencc/opencc-jieba-linux-x64',
  '@opencc/opencc-jieba-win32-x64',
];

function fail(message) {
  console.error(message);
  process.exitCode = 1;
}

for (const relativePath of requiredFiles) {
  const filePath = path.join(packageRoot, relativePath);
  if (!fs.existsSync(filePath)) {
    fail(`Missing required package file: ${relativePath}`);
  }
}

for (const packageName of expectedOptionalPackages) {
  const version = packageJson.optionalDependencies &&
    packageJson.optionalDependencies[packageName];
  if (!version) {
    fail(
      `Missing optional dependency ${packageName}`
    );
  }
}

if (process.exitCode) {
  console.error(
    'opencc-jieba package is not ready. Run npm run build:all before publishing.'
  );
} else {
  console.log(`opencc-jieba@${packageJson.version} package files are ready.`);
}
