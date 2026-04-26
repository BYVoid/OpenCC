const assert = require('assert');
const childProcess = require('child_process');
const fs = require('fs');
const os = require('os');
const path = require('path');
const util = require('util');

const OpenCC = require('./opencc');

const cases = JSON.parse(fs.readFileSync('test/testcases/testcases.json', 'utf-8')).cases || [];

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

  it('converts stdin to stdout', function () {
    const result = childProcess.spawnSync(process.execPath, [cli, '-c', 's2t.json'], {
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

  it('prints help', function () {
    const result = childProcess.spawnSync(process.execPath, [cli, '--help'], {
      encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    assert.match(result.stdout, /Usage:/);
    assert.match(result.stdout, /Unsupported in the npm CLI:/);
    assert.match(result.stdout, /--inspect/);
    assert.match(result.stdout, /--segmentation/);
    assert.match(result.stdout, /plugins/);
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
});
