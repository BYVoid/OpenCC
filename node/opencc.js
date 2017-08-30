/**
 * @file
 * Node.js API.
 *
 * @license
 * Open Chinese Convert
 *
 * Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
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

var path = require('path');
var Promise = require('any-promise');
var bindingPath = require('node-pre-gyp').find(require.resolve('../package.json'));
var binding = require(bindingPath);

var assetsPath = path.dirname(bindingPath);
var getConfigPath = function (config) {
  var configPath = config;
  if (config[0] !== '/' && config[1] !== ':') {
    // Resolve relative path
    configPath = path.join(assetsPath, config);
  }
  return configPath;
};

/**
 * OpenCC Node.js API
 *
 * @class OpenCC
 * @constructor
 * @ingroup node_api
 */
var OpenCC = module.exports = function (config) {
  if (!config) {
    config = 's2t.json';
  }
  config = getConfigPath(config);
  this.handler = new binding.Opencc(config);
};

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
 * @return Converted text.
 * @ingroup node_api
 */
OpenCC.generateDict = binding.Opencc.generateDict.bind(binding.Opencc)

/**
 * Converts input text.
 *
 * @fn void convert(string input, function callback)
 * @memberof OpenCC
 * @param input Input text.
 * @param callback Callback function(err, convertedText).
 * @return The Promise that will yield the converted text.
 * @ingroup node_api
 */
OpenCC.prototype.convert = function (input, callback) {
  input = input.toString()
  var handler = this.handler
  if (callback) {
    return handler.convert(input, callback);
  } else {
    return new Promise(function(resolve, reject) {
      handler.convert(input, function(err, text) {
        if (err) {
          reject(err);
        } else {
          resolve(text);
        }
      });
    });
  }
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
