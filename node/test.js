const assert = require('assert');
const childProcess = require('child_process');
const fs = require('fs');
const nodeGypBuild = require('node-gyp-build');
const os = require('os');
const path = require('path');
const util = require('util');

const OpenCC = require('./opencc');
const { prepareArtifacts } = require('../scripts/prepare-node-prebuild-artifacts');

const cases = JSON.parse(fs.readFileSync('test/testcases/testcases.json', 'utf-8')).cases || [];

function createLocalInstalledShape() {
  const rootDir = path.resolve(__dirname, '..');
  const jiebaPackageDir = path.join(rootDir, 'plugins', 'jieba', 'node');
  const libName = os.platform() === 'win32' ? 'opencc-jieba.dll'
    : os.platform() === 'darwin' ? 'libopencc-jieba.dylib'
      : 'libopencc-jieba.so';
  const requiredFiles = [
    path.join(jiebaPackageDir, 'index.js'),
    path.join(jiebaPackageDir, 'data', 's2twp_jieba.json'),
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

const testSync = function (tc, cfg, expected, done) {
  const opencc = new OpenCC(cfg + '.json');
  const converted = opencc.convertSync(tc.input);
  assert.equal(converted, expected);
  done();
};

const testAsync = function (tc, cfg, expected, done) {
  const opencc = new OpenCC(cfg + '.json');
  opencc.convert(tc.input, function (err, converted) {
    if (err) return done(err);
    assert.equal(converted, expected);
    done();
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
      it('[' + cfg + '] case #' + (idx + 1), function (done) {
        testSync(tc, cfg, expected, done);
      });
    });
  });
});

describe('Async API', function () {
  cases.forEach(function (tc, idx) {
    Object.entries(tc.expected || {}).forEach(function ([cfg, expected]) {
      it('[' + cfg + '] case #' + (idx + 1), function (done) {
        testAsync(tc, cfg, expected, done);
      });
    });
  });
});

describe('Async Promise API', function () {
  cases.forEach(function (tc, idx) {
    Object.entries(tc.expected || {}).forEach(function ([cfg, expected]) {
      it('[' + cfg + '] case #' + (idx + 1), function (done) {
        testAsyncPromise(tc, cfg, expected).then(() => done(), done);
      });
    });
  });
});

describe('npm CLI', function () {
  const cli = path.join(__dirname, 'cli.js');

  function getAssetsPath() {
    const bindingPath = nodeGypBuild.path(path.join(__dirname, '..'));
    const bindingDir = path.dirname(bindingPath);
    const prebuildsDir = path.dirname(bindingDir);
    if (path.basename(prebuildsDir) === 'prebuilds') {
      const sharedAssetsPath = path.join(prebuildsDir, 'assets');
      if (fs.existsSync(sharedAssetsPath)) {
        return sharedAssetsPath;
      }
    }
    return bindingDir;
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
    fs.copyFileSync(path.join(assetsPath, 'STPhrases.ocd2'), path.join(dir, 'STPhrases.ocd2'));
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
  it('loads jieba configs by mode name in the JavaScript API', function () {
    const installRoot = createLocalInstalledShape();
    if (!installRoot) this.skip();

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

  it('loads jieba configs by mode name in the npm CLI', function () {
    const installRoot = createLocalInstalledShape();
    if (!installRoot) this.skip();

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
