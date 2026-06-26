const assert = require('assert');
const childProcess = require('child_process');
const fs = require('fs');
const os = require('os');
const path = require('path');
const { after, describe, it } = require('node:test');

const OpenCC = require('./opencc');
const { prepareArtifacts } = require('../scripts/prepare-node-prebuild-artifacts');

const parseJSON = OpenCC._parseJSON;

const cases = parseJSON(fs.readFileSync('test/testcases/testcases.json', 'utf-8')).cases || [];

function createLocalInstalledShape() {
  const rootDir = path.resolve(__dirname, '..');
  const jiebaPackageDir = path.join(rootDir, 'plugins', 'jieba', 'node');
  const libName = os.platform() === 'win32' ? 'opencc-jieba.dll'
    : os.platform() === 'darwin' ? 'libopencc-jieba.dylib'
      : 'libopencc-jieba.so';
  const requiredFiles = [
    path.join(jiebaPackageDir, 'index.js'),
    path.join(jiebaPackageDir, 'data', 's2twp_jieba.json'),
    path.join(jiebaPackageDir, 'data', 'tw2sp_jieba.json'),
    path.join(jiebaPackageDir, 'data', 'jieba_dict', 'jieba_merged.ocd2'),
    path.join(jiebaPackageDir, 'prebuilds', `${os.platform()}-${os.arch()}`, libName),
  ];
  if (!requiredFiles.every((file) => fs.existsSync(file))) {
    return null;
  }
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'opencc-node-jieba-'));
  const nodeModulesDir = path.join(root, 'node_modules');
  fs.mkdirSync(nodeModulesDir, { recursive: true });
  fs.symlinkSync(rootDir, path.join(nodeModulesDir, 'opencc'), 'dir');
  fs.symlinkSync(jiebaPackageDir, path.join(nodeModulesDir, 'opencc-jieba'), 'dir');
  return root;
}

const testSync = function (tc, cfg, expected) {
  const opencc = new OpenCC(cfg + '.json');
  const converted = opencc.convertSync(tc.input);
  assert.equal(converted, expected);
};

const testAsync = function (tc, cfg, expected) {
  return new Promise(function (resolve, reject) {
    const opencc = new OpenCC(cfg + '.json');
    opencc.convert(tc.input, function (err, converted) {
      if (err) return reject(err);
      try {
        assert.equal(converted, expected);
        resolve();
      } catch (e) {
        reject(e);
      }
    });
  });
};

async function testAsyncPromise(tc, cfg, expected) {
  const opencc = new OpenCC(cfg + '.json');
  const converted = await opencc.convertPromise(tc.input);
  assert.equal(converted, expected);
}

describe('Sync API', function () {
  cases.forEach(function (tc, idx) {
    Object.entries(tc.expected || {}).forEach(function ([cfg, expected]) {
      it('[' + cfg + '] case #' + (idx + 1), function () {
        testSync(tc, cfg, expected);
      });
    });
  });
});

describe('API compatibility', function () {
  it('includes tofu-risk dictionaries by default', function () {
    const opencc = new OpenCC('t2s.json');
    assert.equal(opencc.convertSync('㑮'), '𫝈');
  });

  it('supports JSONC (JSON with comments and trailing commas) configuration files', function () {
    const tempConfigPath = path.join(os.tmpdir(), 'test_comment_config.json');
    fs.writeFileSync(tempConfigPath, `
      // This is a single line comment
      {
        "name": "Test Config", /* This is a multi-line
        comment */
        "segmentation": {
          "type": "mmseg",
          "dict": {
            "type": "inline",
            "entries": {},
          },
        },
        "conversion_chain": [{
          "dict": {
            "type": "inline",
            "entries": {
              "A": "B",
            },
          },
        }],
      }
    `);
    try {
      const opencc = new OpenCC(tempConfigPath, { includeTofuRiskDictionaries: false });
      assert.equal(opencc.convertSync('A'), 'B');

      const openccWithTofu = new OpenCC(tempConfigPath, { includeTofuRiskDictionaries: true });
      assert.equal(openccWithTofu.convertSync('A'), 'B');
    } finally {
      if (fs.existsSync(tempConfigPath)) {
        fs.unlinkSync(tempConfigPath);
      }
    }
  });
});

describe('Async API', function () {
  cases.forEach(function (tc, idx) {
    Object.entries(tc.expected || {}).forEach(function ([cfg, expected]) {
      it('[' + cfg + '] case #' + (idx + 1), function () {
        return testAsync(tc, cfg, expected);
      });
    });
  });
});

describe('Async Promise API', function () {
  cases.forEach(function (tc, idx) {
    Object.entries(tc.expected || {}).forEach(function ([cfg, expected]) {
      it('[' + cfg + '] case #' + (idx + 1), function () {
        return testAsyncPromise(tc, cfg, expected);
      });
    });
  });
});

describe('npm CLI', function () {
  const cli = path.join(__dirname, 'cli.js');

  // The opencc module already resolves the addon and its adjacent assets dir.
  function getAssetsPath() {
    return OpenCC._assetsPath;
  }

  it('converts stdin to stdout', function () {
    const result = childProcess.spawnSync(process.execPath, [cli, '-c', 's2t.json'], {
      input: '汉字',
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout, '漢字');
  });

  it('preserves stdin line endings and unknown characters', function () {
    const input = Buffer.from('鼠标=mouse\r\n123\n未登录', 'utf8');
    const result = childProcess.spawnSync(process.execPath, [cli, '-c', 's2t.json'], {
      input,
    });
    assert.equal(result.status, 0, result.stderr.toString('utf8'));
    assert.deepEqual(result.stdout, Buffer.from('鼠標=mouse\r\n123\n未登錄', 'utf8'));
  });

  it('appends .json to built-in config names', function () {
    const result = childProcess.spawnSync(process.execPath, [cli, '-c', 's2t'], {
      input: '汉字',
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout, '漢字');
  });

  it('skips tofu-risk dictionaries by default', function () {
    const result = childProcess.spawnSync(process.execPath, [cli, '-c', 't2s.json'], {
      input: '㑮',
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout, '㑮');
  });

  it('includes tofu-risk dictionaries when requested', function () {
    const result = childProcess.spawnSync(process.execPath, [
      cli,
      '-c',
      't2s.json',
      '--include-tofu-risk-dictionaries',
    ], {
      input: '㑮',
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout, '𫝈');
  });

  it('converts input file to output file', function () {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'opencc-node-cli-'));
    const input = path.join(dir, 'input.txt');
    const output = path.join(dir, 'output.txt');
    fs.writeFileSync(input, '汉字', 'utf8');
    const result = childProcess.spawnSync(process.execPath, [
      cli,
      '--config=s2t.json',
      '--input',
      input,
      '--output',
      output,
    ], {
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(fs.readFileSync(output, 'utf8'), '漢字');
  });

  it('preserves input file line endings and unknown characters', function () {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'opencc-node-cli-'));
    const input = path.join(dir, 'input.txt');
    const output = path.join(dir, 'output.txt');
    fs.writeFileSync(input, Buffer.from('鼠标=mouse\r\n123\n未登录', 'utf8'));
    const result = childProcess.spawnSync(process.execPath, [
      cli,
      '--config=s2t.json',
      '--input',
      input,
      '--output',
      output,
    ], {
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.deepEqual(
      fs.readFileSync(output),
      Buffer.from('鼠標=mouse\r\n123\n未登錄', 'utf8')
    );
  });

  it('rejects converting an input file onto itself', function () {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'opencc-node-cli-same-file-'));
    const input = path.join(dir, 'input.txt');
    fs.writeFileSync(input, '汉字', 'utf8');
    const result = childProcess.spawnSync(process.execPath, [
      cli,
      '--config=s2t.json',
      '--input',
      input,
      '--output',
      input,
    ], {
      encoding: 'utf8',
    });
    assert.notEqual(result.status, 0);
    assert.match(result.stderr, /same file/);
    assert.equal(fs.readFileSync(input, 'utf8'), '汉字');
  });

  it('preserves phrase conversion across stream chunk boundaries', function () {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'opencc-node-cli-boundary-'));
    const input = path.join(dir, 'input.txt');
    const output = path.join(dir, 'output.txt');
    fs.writeFileSync(input, 'a'.repeat(65535) + '后台老板', 'utf8');
    const result = childProcess.spawnSync(process.execPath, [
      cli,
      '--config=s2t.json',
      '--input',
      input,
      '--output',
      output,
    ], {
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(fs.readFileSync(output, 'utf8'), 'a'.repeat(65535) + '後臺老闆');
  });

  it('resolves custom relative config paths from cwd', function () {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'opencc-node-cli-config-'));
    const assetsPath = getAssetsPath();
    fs.copyFileSync(path.join(assetsPath, 's2t.json'), path.join(dir, 'custom-s2t.json'));
    fs.copyFileSync(path.join(assetsPath, 'STPhrases_WithGeneratedFromRegionalPhrases.ocd2'), path.join(dir, 'STPhrases_WithGeneratedFromRegionalPhrases.ocd2'));
    fs.copyFileSync(path.join(assetsPath, 'STCharacters.ocd2'), path.join(dir, 'STCharacters.ocd2'));

    const result = childProcess.spawnSync(process.execPath, [cli, '-c', './custom-s2t.json'], {
      cwd: dir,
      input: '汉字',
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout, '漢字');
  });

  it('does not append .json to custom relative config paths', function () {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'opencc-node-cli-config-stem-'));
    const assetsPath = getAssetsPath();
    fs.copyFileSync(path.join(assetsPath, 's2t.json'), path.join(dir, 'custom-s2t.json'));
    fs.copyFileSync(path.join(assetsPath, 'STPhrases.ocd2'), path.join(dir, 'STPhrases.ocd2'));
    fs.copyFileSync(path.join(assetsPath, 'STCharacters.ocd2'), path.join(dir, 'STCharacters.ocd2'));

    const result = childProcess.spawnSync(process.execPath, [cli, '-c', './custom-s2t'], {
      cwd: dir,
      input: '汉字',
      encoding: 'utf8',
    });
    assert.notEqual(result.status, 0);
    assert.match(result.stderr, /custom-s2t/);
  });

  it('prints help', function () {
    const result = childProcess.spawnSync(process.execPath, [cli, '--help'], {
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.match(result.stdout, /Usage:/);
    assert.match(result.stdout, /Unsupported in the npm CLI:/);
    assert.match(result.stdout, /--inspect/);
    assert.match(result.stdout, /--segmentation/);
  });

  it('prints version', function () {
    const result = childProcess.spawnSync(process.execPath, [cli, '--version'], {
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout.trim(), OpenCC.version);
  });

  it('rejects unsupported diagnostic modes', function () {
    const inspect = childProcess.spawnSync(process.execPath, [cli, '--inspect'], {
      encoding: 'utf8',
    });
    assert.notEqual(inspect.status, 0);
    assert.match(inspect.stderr, /not supported/);

    const segmentation = childProcess.spawnSync(process.execPath, [cli, '--segmentation'], {
      encoding: 'utf8',
    });
    assert.notEqual(segmentation.status, 0);
    assert.match(segmentation.stderr, /not supported/);
  });

  it('rejects empty inline option values', function () {
    for (const option of ['--config=', '--input=', '--output=']) {
      const result = childProcess.spawnSync(process.execPath, [cli, option], {
        input: '汉字',
        encoding: 'utf8',
      });
      assert.notEqual(result.status, 0);
      assert.match(result.stderr, /Missing value/);
    }
  });

  describe('Line ending preservation', function () {
    const tempDir = fs.mkdtempSync(path.join(os.tmpdir(), 'opencc-line-'));

    function hasCRLF(filePath) {
      const content = fs.readFileSync(filePath, 'utf8');
      // Check for CRLF pattern in the file
      return /\r\n/.test(content);
    }

    function hasLFOnly(filePath) {
      const content = fs.readFileSync(filePath, 'utf8');
      // Check for LF without CR
      return /\n/.test(content) && !/\r/.test(content);
    }

    after(function () {
      // Cleanup temp directory
      try {
        const entries = fs.readdirSync(tempDir);
        for (const entry of entries) {
          fs.rmSync(path.join(tempDir, entry), { recursive: true, force: true });
        }
        fs.rmdirSync(tempDir);
      } catch (e) {
        // Ignore cleanup errors
      }
    });

    it('preserves LF line ending', function () {
      const input = path.join(tempDir, 'input_lf.txt');
      const output = path.join(tempDir, 'output_lf.txt');
      fs.writeFileSync(input, '第一行\n第二行\n', 'utf8');

      const result = childProcess.spawnSync(process.execPath, [
        cli,
        '-c', 's2t.json',
        '--input', input,
        '--output', output,
      ], {
        encoding: 'utf8',
      });

      assert.equal(result.status, 0, result.stderr);
      assert(hasLFOnly(output), 'LF line ending should be preserved');
    });

    it('preserves CRLF line ending', function () {
      const input = path.join(tempDir, 'input_crlf.txt');
      const output = path.join(tempDir, 'output_crlf.txt');
      fs.writeFileSync(input, '第一行\r\n第二行\r\n', 'utf8');

      const result = childProcess.spawnSync(process.execPath, [
        cli,
        '-c', 's2t.json',
        '--input', input,
        '--output', output,
      ], {
        encoding: 'utf8',
      });

      assert.equal(result.status, 0, result.stderr);
      assert(hasCRLF(output), 'CRLF line ending should be preserved');
    });

    it('preserves CRLF with ASCII content', function () {
      const input = path.join(tempDir, 'input_ascii_crlf.txt');
      const output = path.join(tempDir, 'output_ascii_crlf.txt');
      fs.writeFileSync(input, 'hello\r\nworld\r\n', 'utf8');

      const result = childProcess.spawnSync(process.execPath, [
        cli,
        '-c', 's2t.json',
        '--input', input,
        '--output', output,
      ], {
        encoding: 'utf8',
      });

      assert.equal(result.status, 0, result.stderr);
      assert(hasCRLF(output), 'CRLF line ending should be preserved for ASCII');
    });

    it('preserves LF when no trailing newline', function () {
      const input = path.join(tempDir, 'input_no_trailing.txt');
      const output = path.join(tempDir, 'output_no_trailing.txt');
      fs.writeFileSync(input, '第一行\n第二行', 'utf8');

      const result = childProcess.spawnSync(process.execPath, [
        cli,
        '-c', 's2t.json',
        '--input', input,
        '--output', output,
      ], {
        encoding: 'utf8',
      });

      assert.equal(result.status, 0, result.stderr);
      // Input had LF, so output should have LF (no CR)
      assert(!/\r/.test(fs.readFileSync(output, 'utf8')), 'No CR in output');
    });
  });
});

describe('Optional opencc-jieba package integration', function () {
  it('loads jieba configs by mode name in the JavaScript API', function (t) {
    const installRoot = createLocalInstalledShape();
    if (!installRoot) {
      t.skip();
      return;
    }

    const script = [
      "const OpenCC = require('opencc');",
      "const converter = new OpenCC('s2twp_jieba');",
      "process.stdout.write(converter.convertSync('云计算'));",
    ].join('');
    const result = childProcess.spawnSync(process.execPath, ['-e', script], {
      cwd: installRoot,
      env: { ...process.env },
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout, '雲端計算');
  });

  it('loads jieba configs by mode name in the npm CLI', function (t) {
    const installRoot = createLocalInstalledShape();
    if (!installRoot) {
      t.skip();
      return;
    }

    const result = childProcess.spawnSync(process.execPath, [
      path.join(installRoot, 'node_modules', 'opencc', 'node', 'cli.js'),
      '-c',
      's2twp_jieba',
    ], {
      cwd: installRoot,
      env: { ...process.env },
      input: '云计算',
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout, '雲端計算');
  });

  it('skips tofu-risk dictionaries in jieba configs by default in the npm CLI', function (t) {
    const installRoot = createLocalInstalledShape();
    if (!installRoot) {
      t.skip();
      return;
    }

    const result = childProcess.spawnSync(process.execPath, [
      path.join(installRoot, 'node_modules', 'opencc', 'node', 'cli.js'),
      '-c',
      'tw2sp_jieba',
    ], {
      cwd: installRoot,
      env: { ...process.env },
      input: '㑮',
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout, '㑮');
  });

  it('includes tofu-risk dictionaries in jieba configs when requested in the npm CLI', function (t) {
    const installRoot = createLocalInstalledShape();
    if (!installRoot) {
      t.skip();
      return;
    }

    const result = childProcess.spawnSync(process.execPath, [
      path.join(installRoot, 'node_modules', 'opencc', 'node', 'cli.js'),
      '-c',
      'tw2sp_jieba',
      '--include-tofu-risk-dictionaries',
    ], {
      cwd: installRoot,
      env: { ...process.env },
      input: '㑮',
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.equal(result.stdout, '𫝈');
  });
});

describe('Node prebuild assets', function () {
  it('collects only runtime json and ocd2 assets', function () {
    const root = fs.mkdtempSync(path.join(os.tmpdir(), 'opencc-prebuild-assets-'));
    const releaseDir = path.join(root, 'build', 'Release');
    const assetsDir = path.join(root, 'prebuilds', 'assets');

    fs.mkdirSync(releaseDir, { recursive: true });
    fs.mkdirSync(assetsDir, { recursive: true });
    fs.writeFileSync(path.join(releaseDir, 's2t.json'), '{}');
    fs.writeFileSync(path.join(releaseDir, 'STCharacters.ocd2'), 'dict');
    fs.writeFileSync(path.join(releaseDir, 'STCharacters.txt'), 'source');
    fs.writeFileSync(path.join(releaseDir, 'README.md'), 'docs');
    fs.writeFileSync(path.join(assetsDir, 'stale.txt'), 'stale');

    prepareArtifacts(root);

    assert.deepEqual(fs.readdirSync(assetsDir).sort(), [
      'STCharacters.ocd2',
      's2t.json',
    ]);
  });
});
