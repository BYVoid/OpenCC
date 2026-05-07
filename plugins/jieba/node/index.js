const path = require('path');
const os = require('os');

const platform = os.platform();
const arch = os.arch();
const prebuildDir = path.join(__dirname, 'prebuilds', `${platform}-${arch}`);

// Platform-specific plugin library filename
const libName = platform === 'win32' ? 'opencc-jieba.dll'
  : platform === 'darwin' ? 'libopencc-jieba.dylib'
  : 'libopencc-jieba.so';

module.exports = {
  /** Directory containing the prebuilt libopencc-jieba shared library. */
  pluginDir: prebuildDir,
  /** Absolute path to the platform-specific plugin shared library. */
  pluginLibrary: path.join(prebuildDir, libName),
  /** Root data directory (config files and jieba_dict/ live here). */
  dataDir: path.join(__dirname, 'data'),
};
