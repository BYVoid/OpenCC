#!/usr/bin/env node

const childProcess = require('child_process');
const fs = require('fs');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const scopedPackageName = `@opencc/opencc-${process.platform}-${process.arch}`;
const isSourceCheckout = fs.existsSync(path.join(packageRoot, '.git'));

// In a source checkout, the native addon is built with Bazel, not node-gyp
// (see CONTRIBUTING.md). Skip the npm-install source build entirely.
if (isSourceCheckout) {
  console.log(
    'opencc: source checkout detected; skipping the node-gyp install build.\n' +
    'opencc: run ./scripts/build-node-prebuild-bazel.sh to build the addon ' +
    'with Bazel before `npm test`.'
  );
  process.exit(0);
}

try {
  const scopedPackage = require(require.resolve(scopedPackageName, {
    paths: [packageRoot],
  }));
  if (scopedPackage && scopedPackage.binaryPath) {
    process.exit(0);
  }
} catch (error) {
  if (!error || error.code !== 'MODULE_NOT_FOUND') {
    throw error;
  }
}

const nodeGypBuildBin = require.resolve('node-gyp-build/bin.js');
const result = childProcess.spawnSync(process.execPath, [nodeGypBuildBin], {
  cwd: packageRoot,
  stdio: 'inherit',
});

process.exit(result.status === null ? 1 : result.status);
