#!/usr/bin/env node

// npm install script: make sure the native addon is available.
//
// 1. In a source checkout, skip: developers build with Bazel explicitly
//    (see CONTRIBUTING.md). `npm install --build-from-source` overrides.
// 2. If the matching @opencc/opencc-<platform>-<arch> scoped binary package
//    is installed, nothing to do.
// 3. Otherwise compile the addon from source with Bazel and stage it under
//    prebuilds/<platform>-<arch>/opencc.node. Only //node:opencc is built:
//    the config/dictionary assets ship prebuilt in the npm tarball
//    (prebuilds/assets), so the source build needs no Python and generates
//    no dictionaries.

const childProcess = require('child_process');
const fs = require('fs');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const target = `${process.platform}-${process.arch}`;
const scopedPackageName = `@opencc/opencc-${target}`;
const isSourceCheckout = fs.existsSync(path.join(packageRoot, '.git'));
const runInShell = process.platform === 'win32';

const buildFromSource = !['', 'false', '0'].includes(
  String(process.env.npm_config_build_from_source || '').toLowerCase()
);

if (isSourceCheckout && !buildFromSource) {
  console.log(
    'opencc: source checkout detected; skipping the install-time build.\n' +
    'opencc: run ./scripts/build-node-prebuild-bazel.sh to build the addon ' +
    'and assets with Bazel before `npm test`,\n' +
    'opencc: or re-run with `npm install --build-from-source` to build the ' +
    'addon here.'
  );
  process.exit(0);
}

// An installed scoped binary package satisfies the install, but never when
// build-from-source explicitly asks for a local compile.
if (!buildFromSource) {
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
}

function commandWorks(command, args) {
  const result = childProcess.spawnSync(command, args, {
    stdio: 'ignore',
    shell: runInShell,
  });
  return !result.error && result.status === 0;
}

// Prefer a Bazel already on PATH; otherwise fetch bazelisk through npx.
// bazelisk downloads the Bazel version pinned in .bazelversion.
function findBazel() {
  for (const command of ['bazel', 'bazelisk']) {
    if (commandWorks(command, ['--version'])) {
      return { command, prefixArgs: [] };
    }
  }
  if (commandWorks('npx', ['--version'])) {
    console.log(
      'opencc: bazel/bazelisk not found on PATH; using `npx @bazel/bazelisk`.'
    );
    return { command: 'npx', prefixArgs: ['--yes', '@bazel/bazelisk'] };
  }
  return null;
}

const bazel = findBazel();
if (!bazel) {
  console.error(
    `opencc: no prebuilt binary for ${target} and Bazel is not available.\n` +
    'opencc: install bazelisk (e.g. `npm install -g @bazel/bazelisk`, see ' +
    'https://bazel.build/install/bazelisk) and a C++ toolchain,\n' +
    'opencc: then re-run npm install.'
  );
  process.exit(1);
}

console.log(
  `opencc: no prebuilt binary for ${target}; building the native addon ` +
  'from source with Bazel...'
);
const build = childProcess.spawnSync(
  bazel.command,
  // --max_idle_secs is a startup option: let the Bazel server exit shortly
  // after the install instead of lingering as a daemon.
  [...bazel.prefixArgs, '--max_idle_secs=10', 'build', '-c', 'opt', '//node:opencc'],
  { cwd: packageRoot, stdio: 'inherit', shell: runInShell }
);
if (build.error || build.status !== 0) {
  console.error('opencc: Bazel build of //node:opencc failed.');
  process.exit(build.status === null || build.status === undefined ? 1 : build.status);
}

const builtAddon = path.join(packageRoot, 'bazel-bin', 'node', 'opencc.node');
if (!fs.existsSync(builtAddon)) {
  console.error(`opencc: expected Bazel output missing: ${builtAddon}`);
  process.exit(1);
}

const destDir = path.join(packageRoot, 'prebuilds', target);
const dest = path.join(destDir, 'opencc.node');
fs.mkdirSync(destDir, { recursive: true });
fs.rmSync(dest, { force: true });
fs.copyFileSync(builtAddon, dest);
// Bazel outputs are read-only; make the staged copy writable for reinstalls.
fs.chmodSync(dest, 0o755);
console.log(`opencc: staged source-built addon at prebuilds/${target}/opencc.node`);
