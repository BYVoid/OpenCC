/**
 * @file
 * Example of Node.js API.
 *
 * @license
 * Open Chinese Convert
 *
 * Copyright 2010-2026 Carbo Kuo and contributors
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

import { OpenCC, OpenCCConfig } from './opencc';

declare const process: {
  exitCode?: number;
};

const input = '汉字';

async function main(): Promise<void> {
  console.log('OpenCC version', OpenCC.version);

  const converter = new OpenCC('s2t.json');

  console.log('Sync API:', converter.convertSync(input));

  const callbackResult = await new Promise<string>((resolve, reject) => {
    converter.convert(input, (err: string | undefined, converted: string) => {
      if (err) {
        reject(new Error(err));
        return;
      }
      resolve(converted);
    });
  });
  console.log('Callback API:', callbackResult);

  console.log('Promise API:', await converter.convertPromise(input));

  const config: OpenCCConfig = {
    name: 'Demo Inline Config',
    conversion_chain: [
      {
        dict: {
          type: 'inline',
          entries: {
            '鼠标': '滑鼠',
            '软件': '軟體',
          },
        },
      },
    ],
  };

  const inlineConverter = OpenCC.fromConfig(config);
  console.log('Inline config:', inlineConverter.convertSync('鼠标和软件'));
}

main().catch((err: unknown) => {
  console.error(err);
  process.exitCode = 1;
});
