import OpenCCDefault, { OpenCC } from './opencc.mjs';

if (OpenCCDefault !== OpenCC) {
  throw new Error('ESM default export does not match named OpenCC export');
}

const converter = new OpenCC('s2t.json');
const result = await converter.convertPromise('汉字');

if (result !== '漢字') {
  throw new Error(`Unexpected ESM conversion result: ${result}`);
}
