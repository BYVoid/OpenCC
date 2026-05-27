const path = require('path');
const os = require('os');

const platform = os.platform();
const arch = os.arch();
const prebuildDir = path.join(__dirname, 'prebuilds', `${platform}-${arch}`);

// Platform-specific plugin library filename
const libName = platform === 'win32' ? 'opencc-jieba.dll'
  : platform === 'darwin' ? 'libopencc-jieba.dylib'
  : 'libopencc-jieba.so';

const dataDir = path.join(__dirname, 'data');
const configDir = dataDir;

function resolveConfigPath(config) {
  if (typeof config !== 'string' || path.isAbsolute(config)) {
    return null;
  }
  const candidates = [config];
  if (!config.endsWith('.json')) {
    candidates.push(`${config}.json`);
  }
  for (const candidate of candidates) {
    if (candidate.includes('/') || candidate.includes('\\')) {
      continue;
    }
    const configPath = path.join(configDir, candidate);
    if (require('fs').existsSync(configPath)) {
      return configPath;
    }
  }
  return null;
}

module.exports = {
  /** Directory containing the prebuilt libopencc-jieba shared library. */
  pluginDir: prebuildDir,
  /** Absolute path to the platform-specific plugin shared library. */
  pluginLibrary: path.join(prebuildDir, libName),
  /** Root data directory (config files and jieba_dict/ live here). */
  dataDir,
  /** Directory containing jieba-backed OpenCC configuration JSON files. */
  configDir,
  /** Resolve a jieba-backed config name, with or without the .json suffix. */
  resolveConfigPath,
};
