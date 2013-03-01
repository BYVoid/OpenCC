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
  var handler = new binding.Opencc(config);

  this.convert = function (input) {
    return handler.convert(input.toString());
  };

  this.setConversionMode = function (conversionMode) {
    return handler.setConversionMode(conversionMode);
  };
};

OpenCC.CONVERSION_FAST = 0;
OpenCC.CONVERSION_SEGMENT_ONLY = 1;
OpenCC.CONVERSION_LIST_CANDIDATES = 2;

