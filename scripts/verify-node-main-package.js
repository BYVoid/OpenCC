#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const packageJson = require(path.join(packageRoot, 'package.json'));

// The npm tarball ships only the JavaScript API, CLI, typings, and the
// prebuilt config/dictionary assets. The native addon comes exclusively from
// the @opencc/opencc-<platform>-<arch> scoped packages (built with Bazel);
// there is no source-build fallback.
const requiredFiles = [
  'node/cli.js',
  'node/opencc.js',
  'node/opencc.mjs',
  'node/opencc.d.ts',
  'node/opencc.d.cts',
  'node/opencc.d.mts',
  'prebuilds/assets/s2t.json',
  'prebuilds/assets/s2twp.json',
  'prebuilds/assets/CJK_Compatibility_Ideographs.ocd2',
  'prebuilds/assets/STCharacters.ocd2',
  'prebuilds/assets/STPhrases.ocd2',
  'prebuilds/assets/STPhrases_GeneratedFromRegionalPhrases.ocd2',
  'prebuilds/assets/TWPhrases.ocd2',
  'prebuilds/assets/TWVariants.ocd2',
  'prebuilds/assets/TWVariantsPhrases.ocd2',
];

const expectedOptionalPackages = [
  '@opencc/opencc-darwin-arm64',
  '@opencc/opencc-darwin-x64',
  '@opencc/opencc-linux-arm64',
  '@opencc/opencc-linux-x64',
  '@opencc/opencc-win32-x64',
];

function fail(message) {
  console.error(message);
  process.exitCode = 1;
}

function isPackedByFilesWhitelist(relativePath, filesList) {
  for (const pattern of filesList) {
    if (pattern === relativePath) {
      return true;
    }
    if (!pattern.includes('*') && relativePath.startsWith(`${pattern.replace(/\/$/, '')}/`)) {
      return true;
    }
    if (pattern.includes('*')) {
      const regex = new RegExp(
        `^${pattern.split('*').map((part) =>
          part.replace(/[.+?^${}()|[\]\\]/g, '\\$&')
        ).join('[^/]*')}$`
      );
      if (regex.test(relativePath)) {
        return true;
      }
    }
  }
  return false;
}

for (const relativePath of requiredFiles) {
  const filePath = path.join(packageRoot, relativePath);
  if (!fs.existsSync(filePath)) {
    fail(`Missing required package file: ${relativePath}`);
  }
  if (!isPackedByFilesWhitelist(relativePath, packageJson.files || [])) {
    fail(
      `Required file ${relativePath} is not covered by the "files" ` +
      'whitelist in package.json'
    );
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
    'opencc package is not ready. Run ' +
    './scripts/build-node-prebuild-bazel.sh before publishing.'
  );
} else {
  console.log(`opencc@${packageJson.version} package files are ready.`);
}
