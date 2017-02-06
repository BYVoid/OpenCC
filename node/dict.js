const OpenCC = require('./opencc');

const input = process.argv[2];
const output = process.argv[3];

OpenCC.generateDict(input, output, "text", "ocd");
