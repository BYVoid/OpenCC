/**
 * @file
 * Example of Node.js API.
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
 * @example node/demo.js
 * This is an example of how to use the Node.js API.
 */

// In your project you should replace './opencc' with 'opencc'
const OpenCC = require('./opencc');

console.log('OpenCC version', OpenCC.version);

// Load the default Simplified to Traditional config
const opencc = new OpenCC('s2t.json');

// Sync API
const converted = opencc.convertSync("汉字");
console.log(converted);

// Async API
opencc.convert("汉字", (err, converted) => {
  console.log(err, converted);
});

// Async API with Promise
opencc.convertPromise("汉字").then(converted => {
  console.log(converted);
});
