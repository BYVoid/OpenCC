#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
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
  plugins              Plugin-backed segmentation is not supported by this npm CLI.

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
      options.config = arg.slice('--config='.length);
    } else if (arg === '-i' || arg === '--input') {
      options.input = readOptionValue(args, i, arg);
      i += 1;
    } else if (arg.startsWith('--input=')) {
      options.input = arg.slice('--input='.length);
    } else if (arg === '-o' || arg === '--output') {
      options.output = readOptionValue(args, i, arg);
      i += 1;
    } else if (arg.startsWith('--output=')) {
      options.output = arg.slice('--output='.length);
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

function readInput(inputFileName) {
  if (inputFileName) {
    return fs.readFileSync(inputFileName, 'utf8');
  }
  return fs.readFileSync(0, 'utf8');
}

function writeOutput(outputFileName, text) {
  if (outputFileName) {
    fs.writeFileSync(outputFileName, text, 'utf8');
  } else {
    process.stdout.write(text);
  }
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
    const converter = new OpenCC(options.config);
    const input = readInput(options.input);
    const output = converter.convertSync(input);
    writeOutput(options.output, output);
  } catch (error) {
    const message = error && error.message ? error.message : String(error);
    fail(path.basename(process.argv[1]) + ': ' + message);
  }
}

main();
