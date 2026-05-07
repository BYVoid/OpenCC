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
const nodeGypBuild = require('node-gyp-build');
const bindingPath = nodeGypBuild.path(path.join(__dirname, '..'));
const binding = require(bindingPath);

const getAssetsPath = function (bindingPath) {
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

// Detect optional opencc-jieba plugin package.
// When installed, jieba-based configs (e.g. s2twp_jieba.json) become available.
let jiebaInfo = null;
try {
  jiebaInfo = require('opencc-jieba');
} catch (e) {
  // opencc-jieba not installed — jieba configs won't be available.
}

const assetsPath = getAssetsPath(bindingPath);
const getConfigPath = function (config) {
  let configPath = config;
  if (config[0] !== '/' && config[1] !== ':') {
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
  if (config.segmentation) {
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
  // Absolutify conversion_chain dict file paths
  if (Array.isArray(config.conversion_chain)) {
    for (const step of config.conversion_chain) {
      patchDictPaths(step.dict, mainAssetsDir);
    }
  }
}

/**
 * OpenCC Node.js API
 *
 * @class OpenCC
 * @constructor
 * @ingroup node_api
 */
const OpenCC = module.exports = function (config) {
  if (!config) {
    config = 's2t.json';
  }

  // When opencc-jieba is installed, check if the requested config is a jieba
  // config. If so, load its JSON, patch all paths to absolute, and pass the
  // patched JSON string directly to the C++ layer via NewFromString.
  if (jiebaInfo && config[0] !== '/' && config[1] !== ':') {
    const jiebaConfigPath = path.join(jiebaInfo.dataDir, config);
    if (fs.existsSync(jiebaConfigPath)) {
      const raw = JSON.parse(fs.readFileSync(jiebaConfigPath, 'utf-8'));
      patchConfigPaths(raw, jiebaInfo, assetsPath);
      this.handler = new binding.Opencc(
        JSON.stringify(raw),
        jiebaInfo.dataDir + '/'
      );
      return;
    }
  }

  config = getConfigPath(config);
  this.handler = new binding.Opencc(config);
};

// This is to support both CommonJS and ES module.
OpenCC.OpenCC = OpenCC;

/**
 * The version of OpenCC library.
 *
 * @fn OpenCC.version
 * @memberof OpenCC
 * @ingroup node_api
 */
OpenCC.version = binding.Opencc.version();

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
