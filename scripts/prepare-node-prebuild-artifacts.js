#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

const runtimeAssetPattern = /\.(json|ocd2)$/;

function prepareArtifacts(root = path.join(__dirname, '..')) {
  const sourceDir = path.join(root, 'build', 'Release');
  const outputDir = path.join(root, 'prebuilds', 'assets');

  fs.rmSync(outputDir, { recursive: true, force: true });
  fs.mkdirSync(outputDir, { recursive: true });

  for (const entry of fs.readdirSync(sourceDir, { withFileTypes: true })) {
    if (!entry.isFile() || !runtimeAssetPattern.test(entry.name)) {
      continue;
    }

    fs.copyFileSync(
      path.join(sourceDir, entry.name),
      path.join(outputDir, entry.name)
    );
  }
}

if (require.main === module) {
  prepareArtifacts();
}

module.exports = { prepareArtifacts };
