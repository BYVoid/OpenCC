#!/usr/bin/env node

const { execFileSync } = require('child_process');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const packageJson = require(path.join(packageRoot, 'package.json'));
const optionalDependencies = packageJson.optionalDependencies || {};

function npmViewVersion(packageName, version) {
  const spec = `${packageName}@${version}`;
  try {
    return execFileSync('npm', ['view', spec, 'version'], {
      encoding: 'utf8',
      stdio: ['ignore', 'pipe', 'pipe'],
    }).trim();
  } catch (error) {
    return null;
  }
}

let hasError = false;

for (const [packageName, version] of Object.entries(optionalDependencies)) {
  const publishedVersion = npmViewVersion(packageName, version);
  if (publishedVersion !== version) {
    console.error(
      `Missing published optional dependency: ${packageName}@${version}`
    );
    hasError = true;
  }
}

if (hasError) {
  console.error(
    'Publish the scoped binary packages before publishing opencc-jieba.'
  );
  process.exit(1);
}

console.log(
  `All optional binary packages for opencc-jieba@${packageJson.version} ` +
  'are published.'
);
