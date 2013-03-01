var path = require('path');
var binding = require('../build/Release/binding');

var OpenCC = module.exports = function (config) {
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

var moduleInit = function () {
  var assetsPath = path.resolve(__dirname, '../build/Release');
  binding.init(assetsPath);
};
moduleInit();
