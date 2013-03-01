var path = require('path');
var binding = require('../build/Release/binding');

var assetsPath = path.resolve(__dirname, '../build/Release');
var getConfigPath = function (config) {
  var configPath = config;
  if (config[0] !== '/' && config[1] !== ':') {
    // Resolve relative path
    configPath = path.join(assetsPath, config);
  }
  return configPath;
};

var OpenCC = module.exports = function (config) {
  if (!config) {
    config = 'zhs2zht.ini';
  }
  config = getConfigPath(config);
  this.handler = new binding.Opencc(config);
};

OpenCC.CONVERSION_FAST = 0;
OpenCC.CONVERSION_SEGMENT_ONLY = 1;
OpenCC.CONVERSION_LIST_CANDIDATES = 2;

OpenCC.prototype.convert = function (input, callback) {
  return this.handler.convert(input.toString(), callback);
};

OpenCC.prototype.convertSync = function (input) {
  return this.handler.convertSync(input.toString());
};

OpenCC.prototype.setConversionMode = function (conversionMode) {
  return this.handler.setConversionMode(conversionMode);
};
