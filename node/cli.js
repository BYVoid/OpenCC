#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const { Transform, pipeline } = require('stream');
const OpenCC = require('./opencc');

const BUILT_IN_CONFIGS = [
  ['s2t.json', 'Simplified Chinese to Traditional Chinese'],
  ['t2s.json', 'Traditional Chinese to Simplified Chinese'],
  ['s2tw.json', 'Simplified Chinese to Traditional Chinese (Taiwan Standard)'],
  ['tw2s.json', 'Traditional Chinese (Taiwan Standard) to Simplified Chinese'],
  ['s2hk.json', 'Simplified Chinese to Traditional Chinese (Hong Kong variant)'],
  ['hk2s.json', 'Traditional Chinese (Hong Kong variant) to Simplified Chinese'],
  ['s2twp.json', 'Simplified Chinese to Traditional Chinese (Taiwan Standard) with Taiwanese idiom'],
  ['tw2sp.json', 'Traditional Chinese (Taiwan Standard) to Simplified Chinese with Mainland Chinese idiom'],
  ['tw2t.json', 'Traditional Chinese (Taiwan Standard) to Traditional Chinese (OpenCC Standard)'],
  ['t2tw.json', 'Traditional Chinese (OpenCC Standard) to Taiwan Standard'],
  ['hk2t.json', 'Traditional Chinese (Hong Kong variant) to Traditional Chinese (OpenCC Standard)'],
  ['t2hk.json', 'Traditional Chinese (OpenCC Standard) to Hong Kong variant'],
  ['t2jp.json', 'Traditional Chinese Characters (Kyujitai) to New Japanese Kanji (Shinjitai)'],
  ['jp2t.json', 'New Japanese Kanji (Shinjitai) to Traditional Chinese Characters (Kyujitai) (OpenCC Standard)'],
];
const BUILT_IN_CONFIG_NAMES = new Set(BUILT_IN_CONFIGS.map(([name]) => name));
const BUILT_IN_CONFIG_STEMS = new Set(
  BUILT_IN_CONFIGS.map(([name]) => name.replace(/\.json$/, ''))
);
const OPTIONAL_JIEBA_CONFIGS = new Set([
  's2hk_jieba.json',
  's2t_jieba.json',
  's2tw_jieba.json',
  's2twp_jieba.json',
  'tw2sp_jieba.json',
]);
const OPTIONAL_JIEBA_CONFIG_STEMS = new Set(
  Array.from(OPTIONAL_JIEBA_CONFIGS).map((name) => name.replace(/\.json$/, ''))
);

function printHelp() {
  console.log(`Open Chinese Convert (OpenCC) npm Command Line Tool

Usage:
  opencc [options]

Options:
  -c, --config <file>  Configuration file. Defaults to s2t.json.
  -i, --input <file>   Read original text from <file>. Defaults to stdin.
  -o, --output <file>  Write converted text to <file>. Defaults to stdout.
  -v, --version        Print OpenCC version.
  -h, --help           Print this help.

Unsupported in the npm CLI:
  --inspect            Use the native OpenCC CLI for inspection output.
  --segmentation       Use the native OpenCC CLI for segmentation output.

Built-in Configurations:
${BUILT_IN_CONFIGS.map(([name, description]) => `  ${name.padEnd(11)} ${description}`).join('\n')}
`);
}

function fail(message) {
  console.error(message);
  process.exitCode = 1;
}

function readOptionValue(args, index, option) {
  const value = args[index + 1];
  if (!value || value.startsWith('-')) {
    throw new Error(`Missing value for ${option}`);
  }
  return value;
}

function readInlineOptionValue(arg, option) {
  const value = arg.slice((option + '=').length);
  if (!value) {
    throw new Error(`Missing value for ${option}`);
  }
  return value;
}

function parseArgs(args) {
  const options = {
    config: 's2t.json',
    input: null,
    output: null,
    help: false,
    version: false,
  };

  for (let i = 0; i < args.length; i += 1) {
    const arg = args[i];
    if (arg === '-h' || arg === '--help') {
      options.help = true;
    } else if (arg === '-v' || arg === '--version') {
      options.version = true;
    } else if (arg === '-c' || arg === '--config') {
      options.config = readOptionValue(args, i, arg);
      i += 1;
    } else if (arg.startsWith('--config=')) {
      options.config = readInlineOptionValue(arg, '--config');
    } else if (arg === '-i' || arg === '--input') {
      options.input = readOptionValue(args, i, arg);
      i += 1;
    } else if (arg.startsWith('--input=')) {
      options.input = readInlineOptionValue(arg, '--input');
    } else if (arg === '-o' || arg === '--output') {
      options.output = readOptionValue(args, i, arg);
      i += 1;
    } else if (arg.startsWith('--output=')) {
      options.output = readInlineOptionValue(arg, '--output');
    } else if (arg === '--inspect' || arg === '--segmentation') {
      throw new Error(`${arg} is not supported by the npm CLI. Use the native OpenCC CLI instead.`);
    } else if (arg === '--path' || arg.startsWith('--path=')) {
      throw new Error('--path is not supported by the npm CLI. Pass an explicit config file path instead.');
    } else if (arg === '--noflush' || arg === '--measured_result' || arg.startsWith('--measured_result=')) {
      throw new Error(`${arg.split('=')[0]} is not supported by the npm CLI.`);
    } else {
      throw new Error(`Unknown option: ${arg}`);
    }
  }

  return options;
}

function resolveConfigPath(config) {
  if (BUILT_IN_CONFIG_NAMES.has(config) || OPTIONAL_JIEBA_CONFIGS.has(config) || path.isAbsolute(config)) {
    return config;
  }

  if (!config.endsWith('.json') && BUILT_IN_CONFIG_STEMS.has(config)) {
    return config + '.json';
  }

  if (!config.endsWith('.json') && OPTIONAL_JIEBA_CONFIG_STEMS.has(config)) {
    return config + '.json';
  }

  return path.resolve(process.cwd(), config);
}

function createConverterStream(converter) {
  const nativeStream = converter._createConverterStream();
  let pendingChunk = null;

  return new Transform({
    transform(chunk, encoding, callback) {
      try {
        const input = Buffer.isBuffer(chunk) ? chunk : Buffer.from(chunk, encoding);
        if (pendingChunk === null) {
          pendingChunk = Buffer.from(input);
          callback();
          return;
        }
        const output = nativeStream.convertChunk(pendingChunk);
        pendingChunk = Buffer.from(input);
        callback(null, output);
      } catch (error) {
        callback(error);
      }
    },
    flush(callback) {
      try {
        callback(null, pendingChunk === null
          ? nativeStream.finish()
          : nativeStream.finish(pendingChunk));
      } catch (error) {
        callback(error);
      }
    },
  });
}

function isSameFile(inputFileName, outputFileName) {
  if (!inputFileName || !outputFileName) {
    return false;
  }

  try {
    const inputStat = fs.statSync(inputFileName);
    const outputStat = fs.statSync(outputFileName);
    return inputStat.dev === outputStat.dev && inputStat.ino === outputStat.ino;
  } catch (error) {
    return path.resolve(process.cwd(), inputFileName) ===
      path.resolve(process.cwd(), outputFileName);
  }
}

function convertStream(converter, options, callback) {
  if (isSameFile(options.input, options.output)) {
    throw new Error('Input and output refer to the same file.');
  }

  const input = options.input ? fs.createReadStream(options.input) : process.stdin;
  const output = options.output ? fs.createWriteStream(options.output) : process.stdout;
  pipeline(input, createConverterStream(converter), output, callback);
}

function main() {
  let options;
  try {
    options = parseArgs(process.argv.slice(2));
  } catch (error) {
    fail(error.message);
    return;
  }

  if (options.help) {
    printHelp();
    return;
  }

  if (options.version) {
    console.log(OpenCC.version);
    return;
  }

  try {
    const converter = new OpenCC(resolveConfigPath(options.config));
    convertStream(converter, options, (error) => {
      if (error) {
        const message = error && error.message ? error.message : String(error);
        fail(path.basename(process.argv[1]) + ': ' + message);
      }
    });
  } catch (error) {
    const message = error && error.message ? error.message : String(error);
    fail(path.basename(process.argv[1]) + ': ' + message);
  }
}

main();
