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
  'prebuilds/assets/CJK_Compatibility_Ideographs.ocd2',
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

// The npm tarball must contain every file the node-gyp source build compiles
// or includes, so that installs without a prebuilt binary (issue #1409) can
// fall back to building from source. Walk the gyp source list and its
// #include closure and check each file against the "files" whitelist.
function parseGypSourcesAndIncludeDirs() {
  const gypiPath = path.join(packageRoot, 'node', 'node_opencc.gypi');
  const gypi = fs.readFileSync(gypiPath, 'utf8');
  const sources = [];
  const includeDirs = [];
  for (const match of gypi.matchAll(/"\.\.(\/[^"]*)?"/g)) {
    const relativePath = match[1] ? match[1].slice(1) : '.';
    if (/\.(cpp|cc)$/.test(relativePath)) {
      sources.push(relativePath);
    } else if (
      fs.existsSync(path.join(packageRoot, relativePath)) &&
      fs.statSync(path.join(packageRoot, relativePath)).isDirectory()
    ) {
      includeDirs.push(relativePath);
    }
  }
  return { sources, includeDirs };
}

function collectSourceBuildClosure() {
  const { sources, includeDirs } = parseGypSourcesAndIncludeDirs();
  const closure = new Set();
  const queue = [...sources];
  while (queue.length > 0) {
    const relativePath = queue.pop();
    if (closure.has(relativePath)) {
      continue;
    }
    const filePath = path.join(packageRoot, relativePath);
    if (!fs.existsSync(filePath)) {
      fail(`Source build references a file missing from the repository: ${relativePath}`);
      continue;
    }
    closure.add(relativePath);
    const content = fs.readFileSync(filePath, 'utf8');
    for (const match of content.matchAll(/^\s*#\s*include\s+"([^"]+)"/gm)) {
      const include = match[1];
      const candidates = [
        path.posix.join(path.posix.dirname(relativePath), include),
        ...includeDirs.map((dir) => path.posix.normalize(path.posix.join(dir, include))),
      ];
      for (const candidate of candidates) {
        if (fs.existsSync(path.join(packageRoot, candidate))) {
          queue.push(candidate);
          break;
        }
      }
    }
  }
  return closure;
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

function verifySourceBuildFilesPacked() {
  const filesList = packageJson.files || [];
  const missing = [...collectSourceBuildClosure()]
    .filter((relativePath) => !isPackedByFilesWhitelist(relativePath, filesList))
    .sort();
  for (const relativePath of missing) {
    fail(
      `Source build requires ${relativePath} but it is not covered by ` +
      'the "files" whitelist in package.json'
    );
  }
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

verifySourceBuildFilesPacked();

if (process.exitCode) {
  console.error(
    'opencc package is not ready. Run npm run prebuild before publishing.'
  );
} else {
  console.log(`opencc@${packageJson.version} package files are ready.`);
}
