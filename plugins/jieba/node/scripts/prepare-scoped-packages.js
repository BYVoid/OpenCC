#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const packageRoot = path.resolve(__dirname, '..');
const sourcePrebuildsDir = path.join(packageRoot, 'prebuilds');
const outputRoot = path.join(packageRoot, 'dist', 'scoped-packages');
const parentPackage = require(path.join(packageRoot, 'package.json'));

const libraryNameByPlatform = {
  darwin: 'libopencc-jieba.dylib',
  linux: 'libopencc-jieba.so',
  win32: 'opencc-jieba.dll',
};

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

function preparePackage(dirname) {
  const parsed = parsePlatformArch(dirname);
  if (!parsed) {
    console.warn(`Skipping unexpected prebuild directory: ${dirname}`);
    return null;
  }

  const { platform, arch } = parsed;
  const libraryName = libraryNameByPlatform[platform];
  if (!libraryName) {
    console.warn(`Skipping unsupported platform: ${dirname}`);
    return null;
  }

  const sourceLibrary = path.join(sourcePrebuildsDir, dirname, libraryName);
  if (!fs.existsSync(sourceLibrary)) {
    console.warn(`Skipping ${dirname}; missing ${libraryName}`);
    return null;
  }

  const packageName = `@opencc/opencc-jieba-${dirname}`;
  const packageDir = path.join(outputRoot, packageName);
  const relativeLibraryPath = path.join('prebuilds', dirname, libraryName);

  fs.rmSync(packageDir, { recursive: true, force: true });
  fs.mkdirSync(packageDir, { recursive: true });

  writeJson(path.join(packageDir, 'package.json'), {
    name: packageName,
    version: parentPackage.version,
    description: `${parentPackage.description} (${dirname} binary)`,
    license: parentPackage.license,
    author: parentPackage.author,
    repository: {
      type: 'git',
      url: 'git+https://github.com/BYVoid/OpenCC.git',
      directory: 'plugins/jieba/node',
    },
    main: 'index.js',
    os: [platform],
    cpu: [arch],
    files: [
      'index.js',
      relativeLibraryPath,
    ],
  });

  fs.writeFileSync(path.join(packageDir, 'index.js'), [
    "const path = require('path');",
    '',
    `const pluginLibrary = path.join(__dirname, ${JSON.stringify(relativeLibraryPath)});`,
    '',
    'module.exports = {',
    '  pluginDir: path.dirname(pluginLibrary),',
    '  pluginLibrary,',
    '};',
    '',
  ].join('\n'));

  copyFile(sourceLibrary, path.join(packageDir, relativeLibraryPath));
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
    .filter(entry => entry.isDirectory())
    .map(entry => preparePackage(entry.name))
    .filter(Boolean);

  if (prepared.length === 0) {
    console.error('No scoped binary packages were prepared.');
    process.exit(1);
  }

  console.log('Prepared scoped binary packages:');
  for (const packageDir of prepared) {
    console.log(`  ${path.relative(packageRoot, packageDir)}`);
  }
}

if (require.main === module) {
  main();
}
