import { OpenCC } from './opencc';

const converter = new OpenCC('s2t.json');

async function main() {
  const result = await converter.convertPromise('汉字');
  console.log(result);
}

main();
