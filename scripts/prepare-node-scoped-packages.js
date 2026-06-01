#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const sourcePrebuildsDir = path.join(packageRoot, 'prebuilds');
const outputRoot = path.join(packageRoot, 'dist', 'scoped-packages');
const parentPackage = require(path.join(packageRoot, 'package.json'));

function parsePlatformArch(dirname) {
  const index = dirname.indexOf('-');
  if (index === -1) {
    return null;
  }
  return {
    platform: dirname.slice(0, index),
    arch: dirname.slice(index + 1),
  };
}

function copyFile(src, dest) {
  fs.mkdirSync(path.dirname(dest), { recursive: true });
  fs.copyFileSync(src, dest);
}

function writeJson(filePath, value) {
  fs.writeFileSync(filePath, `${JSON.stringify(value, null, 2)}\n`);
}

function writeReadme(packageDir, packageName, dirname) {
  fs.writeFileSync(path.join(packageDir, 'README.md'), [
    `# ${packageName}`,
    '',
    `Native OpenCC Node.js binary for \`${dirname}\`.`,
    '',
    'This package is installed automatically as an optional dependency of',
    '`opencc`. Most users should install `opencc` instead of installing this',
    'package directly.',
    '',
    '## Contents',
    '',
    '```text',
    `prebuilds/${dirname}/opencc.node`,
    '```',
    '',
    '## Usage',
    '',
    '```javascript',
    `const binary = require('${packageName}');`,
    'console.log(binary.binaryPath);',
    '```',
    '',
    '## License',
    '',
    `${parentPackage.license}`,
    '',
  ].join('\n'));
}

function preparePackage(dirname) {
  const parsed = parsePlatformArch(dirname);
  if (!parsed) {
    console.warn(`Skipping unexpected prebuild directory: ${dirname}`);
    return null;
  }

  const { platform, arch } = parsed;
  const sourceBinary = path.join(sourcePrebuildsDir, dirname, 'opencc.node');
  if (!fs.existsSync(sourceBinary)) {
    console.warn(`Skipping ${dirname}; missing opencc.node`);
    return null;
  }

  const packageName = `@opencc/opencc-${dirname}`;
  const packageDir = path.join(outputRoot, packageName);
  const relativeBinaryPath = path.posix.join('prebuilds', dirname, 'opencc.node');

  fs.rmSync(packageDir, { recursive: true, force: true });
  fs.mkdirSync(packageDir, { recursive: true });

  writeJson(path.join(packageDir, 'package.json'), {
    name: packageName,
    version: parentPackage.version,
    description: `${parentPackage.description} (${dirname} binary)`,
    license: parentPackage.license,
    author: parentPackage.author,
    repository: parentPackage.repository,
    main: 'index.js',
    os: [platform],
    cpu: [arch],
    files: [
      'index.js',
      'README.md',
      relativeBinaryPath,
    ],
  });

  fs.writeFileSync(path.join(packageDir, 'index.js'), [
    "const path = require('path');",
    '',
    `const binaryPath = path.join(__dirname, ${JSON.stringify(relativeBinaryPath)});`,
    '',
    'module.exports = {',
    '  binaryPath,',
    '};',
    '',
  ].join('\n'));

  writeReadme(packageDir, packageName, dirname);
  copyFile(sourceBinary, path.join(packageDir, relativeBinaryPath));
  return packageDir;
}

function main() {
  if (!fs.existsSync(sourcePrebuildsDir)) {
    console.error(`Missing prebuilds directory: ${sourcePrebuildsDir}`);
    process.exit(1);
  }

  fs.rmSync(outputRoot, { recursive: true, force: true });
  fs.mkdirSync(outputRoot, { recursive: true });

  const prepared = fs.readdirSync(sourcePrebuildsDir, { withFileTypes: true })
    .filter(entry => entry.isDirectory() && entry.name !== 'assets')
    .map(entry => preparePackage(entry.name))
    .filter(Boolean);

  if (prepared.length === 0) {
    console.error('No scoped OpenCC binary packages were prepared.');
    process.exit(1);
  }

  console.log('Prepared scoped OpenCC binary packages:');
  for (const packageDir of prepared) {
    console.log(`  ${path.relative(packageRoot, packageDir)}`);
  }
}

if (require.main === module) {
  main();
}
