const assert = require('assert');
const fs = require('fs');
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
