var OpenCC = require('./opencc');

var opencc = new OpenCC();
opencc.setConversionMode(OpenCC.CONVERSION_FAST);

var converted = opencc.convert("汉字");
console.log(converted);
