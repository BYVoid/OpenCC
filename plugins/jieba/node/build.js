const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const os = require('os');

// Go to the repository root
const rootDir = path.resolve(__dirname, '../../..');
process.chdir(rootDir);

let platform = os.platform();
let arch = os.arch();
let bazelFlags = '-c opt';

// E.g., node build.js linux-arm64
if (process.argv[2]) {
  const parts = process.argv[2].split('-');
  if (parts.length === 2) {
    platform = parts[0];
    arch = parts[1];
    if (platform === 'linux') {
      const configArch = arch === 'x64' ? 'x86_64' : arch;
      bazelFlags += ` --config=linux-${configArch}-remote`;
    }
  }
}

// Ensure outputs are downloaded even if config sets minimal
bazelFlags += ' --remote_download_toplevel';

console.log(`Building native addon and dictionary via Bazel for ${platform}-${arch}...`);
try {
  execSync(`bazel build ${bazelFlags} //plugins/jieba:opencc-jieba //plugins/jieba:jieba_merged_dict`, { stdio: 'inherit' });
} catch (err) {
  console.error('Bazel build failed.', err);
  process.exit(1);
}

// Dynamically resolve exact output paths using cquery
const libPathCmd = `bazel cquery ${bazelFlags} --output=files //plugins/jieba:opencc-jieba`;
const libSrcRaw = execSync(libPathCmd, { encoding: 'utf8', stdio: ['pipe', 'pipe', 'ignore'] }).trim().split('\n').pop();
const libSrc = path.join(rootDir, libSrcRaw);

const dictPathCmd = `bazel cquery ${bazelFlags} --output=files //plugins/jieba:jieba_merged_dict`;
const dictSrcRaw = execSync(dictPathCmd, { encoding: 'utf8', stdio: ['pipe', 'pipe', 'ignore'] }).trim().split('\n').pop();
const mergedOcd2 = path.join(rootDir, dictSrcRaw);

const prebuildDir = path.join(__dirname, 'prebuilds', `${platform}-${arch}`);
const dataDir = path.join(__dirname, 'data');
const jiebaDictDir = path.join(dataDir, 'jieba_dict');

// Create directories
fs.mkdirSync(prebuildDir, { recursive: true });
fs.mkdirSync(jiebaDictDir, { recursive: true });

// Determine library name
const libName = platform === 'win32' ? 'opencc-jieba.dll'
  : platform === 'darwin' ? 'libopencc-jieba.dylib'
  : 'libopencc-jieba.so';

function copyForce(src, dest) {
  if (fs.existsSync(dest)) fs.rmSync(dest, { force: true });
  fs.copyFileSync(src, dest);
}

// Copy library
if (fs.existsSync(libSrc)) {
  copyForce(libSrc, path.join(prebuildDir, libName));
  console.log(`Copied ${libName} to prebuilds/${platform}-${arch}/`);
} else {
  console.error(`Library not found: ${libSrc}`);
  process.exit(1);
}

// Copy config JSON files
const configSrcDir = path.join(rootDir, 'plugins/jieba/data/config');
const configs = [
  's2hk_jieba.json',
  's2t_jieba.json',
  's2tw_jieba.json',
  's2twp_jieba.json',
  'tw2sp_jieba.json',
];
configs.forEach(config => {
  copyForce(path.join(configSrcDir, config), path.join(dataDir, config));
  console.log(`Copied ${config} to data/`);
});

// Copy generated dictionary file
if (fs.existsSync(mergedOcd2)) {
  copyForce(mergedOcd2, path.join(jiebaDictDir, 'jieba_merged.ocd2'));
  console.log(`Copied jieba_merged.ocd2 to data/jieba_dict/`);
} else {
  console.error(`Dictionary not found: ${mergedOcd2}`);
  process.exit(1);
}

// Copy cppjieba static dictionary files
const dictSrcDir = path.join(rootDir, 'plugins/jieba/deps/cppjieba/dict');
const dictFiles = ['hmm_model.utf8', 'idf.utf8', 'stop_words.utf8'];
dictFiles.forEach(file => {
  copyForce(path.join(dictSrcDir, file), path.join(jiebaDictDir, file));
  console.log(`Copied ${file} to data/jieba_dict/`);
});

console.log('\nPackage directory tree restored successfully!');
