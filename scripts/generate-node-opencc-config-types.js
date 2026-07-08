#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const { compile } = require('json-schema-to-typescript');

const rootDir =
  process.env.OPENCC_CONFIG_TYPES_ROOT || path.resolve(__dirname, '..');
const schemaPath = path.join(rootDir, 'data/config/opencc_config.schema.json');
const declarationFiles = [
  'node/opencc.d.ts',
  'node/opencc.d.cts',
  'node/opencc.d.mts',
].map((file) => path.join(rootDir, file));

const definitionTitles = {
  segmentation: 'OpenCCSegmentation',
  mmseg_segmentation: 'OpenCCMmsegSegmentation',
  plugin_segmentation: 'OpenCCPluginSegmentation',
  conversion: 'OpenCCConversion',
  dict: 'OpenCCDict',
  file_dict: 'OpenCCFileDict',
  inline_dict: 'OpenCCInlineDict',
  group_dict: 'OpenCCGroupDict',
};

function loadSchema() {
  const schema = JSON.parse(fs.readFileSync(schemaPath, 'utf8'));
  schema.title = 'OpenCCConfig';

  for (const [name, title] of Object.entries(definitionTitles)) {
    schema.definitions[name].title = title;
  }

  schema.definitions.plugin_segmentation.tsType = [
    '{',
    'type: string;',
    'resources?: Record<string, string>;',
    '[key: string]: string | Record<string, string> | undefined;',
    '}',
  ].join(' ');

  return schema;
}

function normalizeGeneratedTypes(types) {
  return types
    .trim()
    .replace(/^[ \t]*\/\*\*[\s\S]*?\*\/\n/gm, '')
    .replace(/^export /gm, '')
    .replace(/^\n+|\n+$/g, '');
}

async function generatedBlock() {
  const generatedTypes = await compile(loadSchema(), 'OpenCCConfig', {
    bannerComment: '',
    maxItems: -1,
    unreachableDefinitions: true,
  });

  return normalizeGeneratedTypes(generatedTypes);
}

function replaceGeneratedBlock(filePath, block) {
  const source = fs.readFileSync(filePath, 'utf8');

  const optionsStart = source.indexOf('\ninterface OpenCCOptions {');
  if (optionsStart !== -1) {
    return `${block}\n${source.slice(optionsStart)}`;
  }

  const originalConfigType = 'type OpenCCConfig = Record<string, unknown>;';
  if (source.startsWith(originalConfigType)) {
    return `${block}${source.slice(originalConfigType.length)}`;
  }

  throw new Error(`Unable to locate generated block in ${filePath}`);
}

async function main() {
  const block = await generatedBlock();
  let hasDiff = false;

  for (const filePath of declarationFiles) {
    const next = replaceGeneratedBlock(filePath, block);
    const current = fs.readFileSync(filePath, 'utf8');
    if (next !== current) {
      hasDiff = true;
      if (process.argv.includes('--check')) {
        console.error(`${path.relative(rootDir, filePath)} is not up to date.`);
      } else {
        fs.writeFileSync(filePath, next);
      }
    }
  }

  if (process.argv.includes('--check') && hasDiff) {
    process.exit(1);
  }
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
