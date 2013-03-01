var OpenCC = require('./opencc');

var opencc = new OpenCC('zhs2zht.ini');
opencc.setConversionMode(OpenCC.CONVERSION_FAST);

var converted = opencc.convertSync("汉字");
console.log(converted);

opencc.convert("汉字", function (err, converted) {
  console.log(converted);
});
