#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const packageJson = require(path.join(packageRoot, 'package.json'));

const requiredFiles = [
  'binding.gyp',
  'node/cli.js',
  'node/opencc.js',
  'node/opencc.mjs',
  'node/opencc.d.ts',
  'node/opencc.d.cts',
  'node/opencc.d.mts',
  'prebuilds/assets/s2t.json',
  'prebuilds/assets/s2twp.json',
  'prebuilds/assets/STCharacters.ocd2',
  'prebuilds/assets/STPhrases.ocd2',
  'prebuilds/assets/STPhrases_GeneratedFromRegionalPhrases.ocd2',
  'prebuilds/assets/TWVariantsPhrases.ocd2',
];

const expectedOptionalPackages = [
  '@opencc/opencc-darwin-arm64',
  '@opencc/opencc-linux-arm64',
  '@opencc/opencc-linux-x64',
  '@opencc/opencc-win32-x64',
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
    fail(`Missing optional dependency ${packageName}`);
  }
}

if (process.exitCode) {
  console.error(
    'opencc package is not ready. Run npm run prebuild before publishing.'
  );
} else {
  console.log(`opencc@${packageJson.version} package files are ready.`);
}
