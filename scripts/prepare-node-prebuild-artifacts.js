#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const runtimeAssetPattern = /\.(json|ocd2)$/;

function copyMatchingFiles(sourceDir, outputDir, predicate) {
  if (!fs.existsSync(sourceDir)) {
    return 0;
  }

  let count = 0;
  for (const entry of fs.readdirSync(sourceDir, { withFileTypes: true })) {
    if (!entry.isFile() || !predicate(entry.name)) {
      continue;
    }

    fs.copyFileSync(
      path.join(sourceDir, entry.name),
      path.join(outputDir, entry.name)
    );
    count += 1;
  }
  return count;
}

function prepareArtifacts(root = path.join(__dirname, '..')) {
  const legacySourceDir = path.join(root, 'build', 'Release');
  const bazelDictionaryDir = path.join(root, 'bazel-bin', 'data', 'dictionary');
  const configDir = path.join(root, 'data', 'config');
  const outputDir = path.join(root, 'prebuilds', 'assets');

  fs.rmSync(outputDir, { recursive: true, force: true });
  fs.mkdirSync(outputDir, { recursive: true });

  if (fs.existsSync(legacySourceDir)) {
    const copied = copyMatchingFiles(
      legacySourceDir,
      outputDir,
      (name) => runtimeAssetPattern.test(name)
    );
    if (copied > 0) {
      return;
    }
  }

  const jsonCount = copyMatchingFiles(
    configDir,
    outputDir,
    (name) => name.endsWith('.json') && !name.endsWith('.schema.json')
  );
  const dictionaryCount = copyMatchingFiles(
    bazelDictionaryDir,
    outputDir,
    (name) => name.endsWith('.ocd2')
  );

  if (jsonCount === 0 || dictionaryCount === 0) {
    throw new Error(
      'Could not prepare Node.js prebuild assets. Run `npm run prebuild` first, ' +
      'or run `bazel build -c opt //data/dictionary:binary_dictionaries` before this script.'
    );
  }
}

if (require.main === module) {
  prepareArtifacts();
}

module.exports = { prepareArtifacts };
