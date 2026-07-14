#!/usr/bin/env node

const childProcess = require('child_process');
const fs = require('fs');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const scopedPackageName = `@opencc/opencc-${process.platform}-${process.arch}`;
const isSourceCheckout = fs.existsSync(path.join(packageRoot, '.git'));

// In a source checkout, the native addon is built with Bazel, not node-gyp
// (see CONTRIBUTING.md), so the npm-install source build is skipped unless
// the standard npm escape hatch is set (`npm install --build-from-source`,
// or the npm_config_build_from_source environment variable, as used by the
// AppVeyor npm packaging job).
const buildFromSource = !['', 'false', '0'].includes(
  String(process.env.npm_config_build_from_source || '').toLowerCase()
);
if (isSourceCheckout && !buildFromSource) {
  console.log(
    'opencc: source checkout detected; skipping the node-gyp install build.\n' +
    'opencc: run ./scripts/build-node-prebuild-bazel.sh to build the addon ' +
    'with Bazel before `npm test`,\n' +
    'opencc: or re-run with `npm install --build-from-source` for the ' +
    'node-gyp build.'
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
