#!/usr/bin/env node

const childProcess = require('child_process');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const scopedPackageName = `@opencc/opencc-${process.platform}-${process.arch}`;

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
