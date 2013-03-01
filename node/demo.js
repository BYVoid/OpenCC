var OpenCC = require('./opencc');

var opencc = new OpenCC('zhs2zht.ini');
opencc.setConversionMode(OpenCC.CONVERSION_FAST);

var converted = opencc.convert("汉字");
console.log(converted);
