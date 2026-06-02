const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const os = require('os');

const rootDir = path.resolve(__dirname, '../../..');
const packageDir = __dirname;

function normalizeArch(value) {
  if (value === 'x64') {
    return 'x64';
  }
  if (value === 'arm64' || value === 'aarch64') {
    return 'arm64';
  }
  return value;
}

function parseTarget(value) {
  if (!value) {
    return {
      platform: os.platform(),
      arch: normalizeArch(os.arch()),
    };
  }

  const index = value.indexOf('-');
  if (index === -1) {
    throw new Error(`Invalid target '${value}'. Expected platform-arch.`);
  }

  return {
    platform: value.slice(0, index),
    arch: normalizeArch(value.slice(index + 1)),
  };
}

function run(command, args, options = {}) {
  execFileSync(command, args, {
    cwd: rootDir,
    stdio: 'inherit',
    ...options,
  });
}

function copyForce(src, dest) {
  fs.mkdirSync(path.dirname(dest), { recursive: true });
  if (fs.existsSync(dest)) {
    fs.rmSync(dest, { force: true });
  }
  fs.copyFileSync(src, dest);
}

function findFile(dir, filename) {
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  for (const entry of entries) {
    const fullPath = path.join(dir, entry.name);
    if (entry.isFile() && entry.name === filename) {
      return fullPath;
    }
    if (entry.isDirectory()) {
      const found = findFile(fullPath, filename);
      if (found) {
        return found;
      }
    }
  }
  return null;
}

const { platform, arch } = parseTarget(process.argv[2]);
const hostPlatform = os.platform();
const hostArch = normalizeArch(os.arch());

if (platform !== hostPlatform || arch !== hostArch) {
  console.error(
    `Refusing to cross-build ${platform}-${arch} on ${hostPlatform}-${hostArch}. ` +
    'Run this script on the target platform.'
  );
  process.exit(1);
}

const target = `${platform}-${arch}`;
const buildDir = path.join(packageDir, 'build', target);
const config = 'Release';
const libName = platform === 'win32' ? 'opencc-jieba.dll'
  : platform === 'darwin' ? 'libopencc-jieba.dylib'
  : 'libopencc-jieba.so';

console.log(`Building opencc-jieba via CMake for ${target}...`);

run('cmake', [
  '-S', rootDir,
  '-B', buildDir,
  '-DCMAKE_BUILD_TYPE=Release',
  '-DBUILD_SHARED_LIBS=OFF',
  '-DBUILD_OPENCC_JIEBA_PLUGIN=ON',
  '-DENABLE_GTEST=OFF',
  '-DENABLE_BENCHMARK=OFF',
]);

run('cmake', [
  '--build', buildDir,
  '--config', config,
  '--target', 'opencc_jieba',
  '--parallel',
]);

const libSrc = findFile(buildDir, libName);
const mergedOcd2 = findFile(buildDir, 'jieba_merged.ocd2');

if (!libSrc) {
  console.error(`Library not found under ${buildDir}: ${libName}`);
  process.exit(1);
}
if (!mergedOcd2) {
  console.error(`Dictionary not found under ${buildDir}: jieba_merged.ocd2`);
  process.exit(1);
}

const prebuildDir = path.join(packageDir, 'prebuilds', target);
const dataDir = path.join(packageDir, 'data');
const jiebaDictDir = path.join(dataDir, 'jieba_dict');

copyForce(libSrc, path.join(prebuildDir, libName));
console.log(`Copied ${libName} to prebuilds/${target}/`);

const configSrcDir = path.join(rootDir, 'plugins/jieba/data/config');
const configs = [
  's2hk_jieba.json',
  's2t_jieba.json',
  's2tw_jieba.json',
  's2twp_jieba.json',
  'tw2sp_jieba.json',
];
for (const configFile of configs) {
  copyForce(path.join(configSrcDir, configFile), path.join(dataDir, configFile));
  console.log(`Copied ${configFile} to data/`);
}

copyForce(mergedOcd2, path.join(jiebaDictDir, 'jieba_merged.ocd2'));
console.log('Copied jieba_merged.ocd2 to data/jieba_dict/');

const dictSrcDir = path.join(rootDir, 'plugins/jieba/deps/cppjieba/dict');
for (const dictFile of ['hmm_model.utf8', 'idf.utf8', 'stop_words.utf8']) {
  copyForce(path.join(dictSrcDir, dictFile), path.join(jiebaDictDir, dictFile));
  console.log(`Copied ${dictFile} to data/jieba_dict/`);
}

console.log('\nPackage directory tree restored successfully!');
