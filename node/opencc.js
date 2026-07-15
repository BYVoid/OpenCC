/**
 * @file
 * Node.js API.
 *
 * @license
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @defgroup node_api Node.js API
 * 
 * Node.js language binding
 */

const path = require('path');
const fs = require('fs');
const packageRoot = path.join(__dirname, '..');

function requireOptionalPackage(packageName) {
  try {
    return require(packageName);
  } catch (error) {
    if (error && error.code === 'MODULE_NOT_FOUND') {
      return null;
    }
    throw error;
  }
}

function resolveBindingPath() {
  // The addon is always named opencc.node and ships either as a local build
  // (build/Release, used by the Bazel test sandbox and local development), as
  // a prebuild under prebuilds/<platform>-<arch>/, or via an
  // @opencc/opencc-<platform>-<arch> scoped package (OpenCC ships a single
  // N-API build per platform). PREBUILDS_ONLY skips the local build/Release
  // directory so tests exercise the shipped prebuild layout.
  const candidates = [];
  if (!process.env.PREBUILDS_ONLY) {
    candidates.push(path.join(packageRoot, 'build', 'Release', 'opencc.node'));
  }
  candidates.push(path.join(
    packageRoot, 'prebuilds', `${process.platform}-${process.arch}`, 'opencc.node'
  ));
  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }
  const scopedBinaryPackage = requireOptionalPackage(
    `@opencc/opencc-${process.platform}-${process.arch}`
  );
  if (scopedBinaryPackage && scopedBinaryPackage.binaryPath) {
    return scopedBinaryPackage.binaryPath;
  }
  throw new Error(
    'Could not locate the opencc native addon (looked in build/Release, ' +
    'prebuilds/, and @opencc/opencc-* scoped packages)'
  );
}

const bindingPath = resolveBindingPath();
const binding = require(bindingPath);

const getAssetsPath = function (bindingPath) {
  const packageAssetsPath = path.join(packageRoot, 'prebuilds', 'assets');
  if (fs.existsSync(packageAssetsPath)) {
    return packageAssetsPath;
  }

  const bindingDir = path.dirname(bindingPath);
  const prebuildsDir = path.dirname(bindingDir);
  if (path.basename(prebuildsDir) === 'prebuilds') {
    const sharedAssetsPath = path.join(prebuildsDir, 'assets');
    if (fs.existsSync(sharedAssetsPath)) {
      return sharedAssetsPath;
    }
  }
  return bindingDir;
};

function requireOptionalPeer(packageName) {
  const searchPaths = [
    process.cwd(),
    path.join(__dirname, '..'),
    __dirname,
  ];
  for (const searchPath of searchPaths) {
    try {
      return require(require.resolve(packageName, { paths: [searchPath] }));
    } catch (e) {
      // Try the next search path.
    }
  }
  return null;
}

// Detect optional opencc-jieba plugin package.
// When installed, jieba-based configs (e.g. s2twp_jieba.json) become available.
const jiebaInfo = requireOptionalPeer('opencc-jieba');

const assetsPath = getAssetsPath(bindingPath);
const isAbsolutePath = function (filePath) {
  return path.isAbsolute(filePath) || /^[A-Za-z]:[\\/]/.test(filePath);
};

const getConfigPath = function (config) {
  let configPath = config;
  if (!isAbsolutePath(config)) {
    // Resolve relative path
    configPath = path.join(assetsPath, config);
  }
  return configPath;
};

/**
 * Recursively resolve all dict "file" paths in a config object to absolute
 * paths under the given base directory.
 * @param {Object} dict - A dict node from the config JSON.
 * @param {string} baseDir - Directory to resolve relative paths against.
 */
function patchDictPaths(dict, baseDir) {
  if (!dict) return;
  if (dict.type === 'group' && Array.isArray(dict.dicts)) {
    for (const d of dict.dicts) {
      patchDictPaths(d, baseDir);
    }
  } else if (dict.file && !path.isAbsolute(dict.file)) {
    dict.file = path.join(baseDir, dict.file);
  }
}

function filterTofuRiskDicts(dict, includeTofuRiskDictionaries) {
  if (!dict) return null;
  if (dict.type === 'inline') {
    return dict;
  }
  if (dict.may_output_tofu && !includeTofuRiskDictionaries) {
    return null;
  }
  if (dict.type === 'group' && Array.isArray(dict.dicts)) {
    dict.dicts = dict.dicts
      .map((d) => filterTofuRiskDicts(d, includeTofuRiskDictionaries))
      .filter(Boolean);
    if (dict.dicts.length === 0) {
      return null;
    }
  }
  return dict;
}

function filterTofuRiskConversionChain(config, includeTofuRiskDictionaries) {
  if (!Array.isArray(config.conversion_chain)) return;
  config.conversion_chain = config.conversion_chain
    .map((step) => {
      if (!step || !step.dict) return step;
      step.dict = filterTofuRiskDicts(step.dict, includeTofuRiskDictionaries);
      return step.dict ? step : null;
    })
    .filter(Boolean);
}

function isConfigObject(config) {
  return config && typeof config === 'object' && !Array.isArray(config);
}

function cloneConfigObject(config) {
  return JSON.parse(JSON.stringify(config));
}

function createFromConfigObject(config, options) {
  if (!isConfigObject(config)) {
    throw new TypeError('OpenCC config must be an object');
  }
  if (options.resourceZip) {
    throw new TypeError('resourceZip is only supported with config file names');
  }

  const raw = cloneConfigObject(config);
  const includeTofuRiskDictionaries =
    options.includeTofuRiskDictionaries !== false;
  filterTofuRiskConversionChain(raw, includeTofuRiskDictionaries);

  const configDirectory = options.configDirectory || assetsPath;
  return new binding.Opencc(
    JSON.stringify(raw),
    configDirectory + (/[\\/]$/.test(configDirectory) ? '' : path.sep)
  );
}

/**
 * Patch all relative paths in a config JSON object to absolute paths.
 *
 * - segmentation resources + __plugin_library → jieba package
 * - conversion_chain dict files → main opencc package assets
 *
 * @param {Object} config - Parsed config JSON object (modified in-place).
 * @param {Object} jieba - Info from the opencc-jieba package.
 * @param {string} mainAssetsDir - Main opencc package assets directory.
 */
function patchConfigPaths(config, jieba, mainAssetsDir) {
  // Inject explicit plugin library path (skips dlopen search in C++)
  if (jieba && config.segmentation) {
    config.segmentation.__plugin_library = jieba.pluginLibrary;
    // Absolutify segmentation resource paths (dict_path, model_path, etc.)
    if (config.segmentation.resources) {
      const res = config.segmentation.resources;
      for (const key of Object.keys(res)) {
        if (!path.isAbsolute(res[key])) {
          res[key] = path.join(jieba.dataDir, res[key]);
        }
      }
    }
  }
  // Absolutify normalization and conversion_chain dict file paths
  if (Array.isArray(config.normalization)) {
    for (const step of config.normalization) {
      patchDictPaths(step.dict, mainAssetsDir);
    }
  }
  if (Array.isArray(config.conversion_chain)) {
    for (const step of config.conversion_chain) {
      patchDictPaths(step.dict, mainAssetsDir);
    }
  }
}

function resolveJiebaConfigPath(config, jieba) {
  if (!jieba || isAbsolutePath(config)) {
    return null;
  }
  if (typeof jieba.resolveConfigPath === 'function') {
    return jieba.resolveConfigPath(config);
  }
  const candidates = [config];
  if (!config.endsWith('.json')) {
    candidates.push(config + '.json');
  }
  for (const candidate of candidates) {
    if (candidate.includes('/') || candidate.includes('\\')) {
      continue;
    }
    const configPath = path.join(jieba.dataDir, candidate);
    if (fs.existsSync(configPath)) {
      return configPath;
    }
  }
  return null;
}

/**
 * OpenCC Node.js API
 *
 * @class OpenCC
 * @constructor
 * @ingroup node_api
 */
const OpenCC = module.exports = function (config, options) {
  if (!config) {
    config = 's2t.json';
  }
  if (!options) {
    options = {};
  }

  if (isConfigObject(config)) {
    this.handler = createFromConfigObject(config, options);
    return;
  }

  const includeTofuRiskDictionaries =
    options.includeTofuRiskDictionaries !== false;

  if (options.resourceZip) {
    this.handler = new binding.Opencc(
      config,
      options.resourceZip,
      includeTofuRiskDictionaries
    );
    return;
  }

  // When opencc-jieba is installed, check if the requested config is a jieba
  // config. If so, load its JSON, patch all paths to absolute, and pass the
  // patched JSON string directly to the C++ layer via NewFromString.
  const jiebaConfigPath = resolveJiebaConfigPath(config, jiebaInfo);
  if (jiebaConfigPath) {
    const raw = parseJSON(fs.readFileSync(jiebaConfigPath, 'utf-8'));
    filterTofuRiskConversionChain(raw, includeTofuRiskDictionaries);
    patchConfigPaths(raw, jiebaInfo, assetsPath);
    this.handler = new binding.Opencc(
      JSON.stringify(raw),
      jiebaInfo.dataDir + '/'
    );
    return;
  }

  config = getConfigPath(config);
  if (!includeTofuRiskDictionaries) {
    const raw = parseJSON(fs.readFileSync(config, 'utf-8'));
    filterTofuRiskConversionChain(raw, includeTofuRiskDictionaries);
    const configDir = path.dirname(config);
    if (raw.segmentation && raw.segmentation.dict) {
      patchDictPaths(raw.segmentation.dict, configDir);
    }
    if (Array.isArray(raw.conversion_chain)) {
      for (const step of raw.conversion_chain) {
        patchDictPaths(step.dict, configDir);
      }
    }
    this.handler = new binding.Opencc(
      JSON.stringify(raw),
      configDir + path.sep
    );
    return;
  }
  this.handler = new binding.Opencc(config);
};

function parseJSON(str) {
  const cleanStr = str.replace(/"(?:[^"\\]|\\.)*"|(\/\/.*|\/\*[\s\S]*?\*\/)|(,\s*(?=[\]}]))/g, (m, g1, g2) => (g1 || g2) ? "" : m);
  return JSON.parse(cleanStr);
}

// This is to support both CommonJS and ES module.
OpenCC.OpenCC = OpenCC;
OpenCC._parseJSON = parseJSON;

// Resolved native addon path and its adjacent assets (config/dictionary)
// directory, exported so tooling/tests can locate them without re-deriving the
// lookup.
OpenCC._bindingPath = bindingPath;
OpenCC._assetsPath = assetsPath;

/**
 * The version of OpenCC library.
 *
 * @fn OpenCC.version
 * @memberof OpenCC
 * @ingroup node_api
 */
OpenCC.version = binding.Opencc.version();

OpenCC.fromConfig = function (config, options) {
  return new OpenCC(config, options);
};

/**
 * Generates dictionary from another format.
 *
 * @fn string generateDict(string inputFileName, string outputFileName, string formatFrom, string formatTo)
 * @memberof OpenCC
 * @param inputFileName Input dictionary filename.
 * @param outputFileName Output dictionary filename.
 * @param formatFrom Input dictionary format.
 * @param formatTo Input dictionary format.
 * @ingroup node_api
 */
OpenCC.generateDict = function (inputFileName, outputFileName,
  formatFrom, formatTo) {
  return binding.Opencc.generateDict(inputFileName, outputFileName,
    formatFrom, formatTo);
}

/**
 * Converts input text.
 *
 * @fn void convert(string input, function callback)
 * @memberof OpenCC
 * @param input Input text.
 * @param callback Callback function(err, convertedText).
 * @ingroup node_api
 */
OpenCC.prototype.convert = function (input, callback) {
  return this.handler.convert(input.toString(), callback);
};

/**
 * Converts input text.
 *
 * @fn string convertSync(string input)
 * @memberof OpenCC
 * @param input Input text.
 * @return Converted text.
 * @ingroup node_api
 */
OpenCC.prototype.convertSync = function (input) {
  return this.handler.convertSync(input.toString());
};

/**
 * Converts input text asynchronously and returns a Promise.
 *
 * @fn Promise convertPromise(string input)
 * @memberof OpenCC
 * @param input Input text.
 * @return The Promise that will yield the converted text.
 * @ingroup node_api
 */
OpenCC.prototype.convertPromise = function (input) {
  const self = this;
  return new Promise(function (resolve, reject) {
    self.handler.convert(input.toString(), function (err, text) {
      if (err) reject(err);
      else resolve(text);
    });
  });
};

OpenCC.prototype._createConverterStream = function () {
  return new binding.OpenccStream(this.handler);
};
