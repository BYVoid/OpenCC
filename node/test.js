const assert = require('assert');
const fs = require('fs');
const util = require('util');

const OpenCC = require('./opencc');

const configs = [
  'hk2s',
  'hk2t',
  'jp2t',
  's2hk',
  's2t',
  's2tw',
  's2twp',
  't2hk',
  't2jp',
  't2s',
  'tw2s',
  'tw2sp',
  'tw2t',
];

const testSync = function (config, done) {
  const inputName = 'test/testcases/' + config + '.in';
  const outputName = 'test/testcases/' + config + '.ans';
  const configName = config + '.json';
  const opencc = new OpenCC(configName);
  const text = fs.readFileSync(inputName, 'utf-8');
  const converted = opencc.convertSync(text);
  const answer = fs.readFileSync(outputName, 'utf-8');
  assert.equal(converted, answer);
  done();
};

const testAsync = function (config, done) {
  const inputName = 'test/testcases/' + config + '.in';
  const outputName = 'test/testcases/' + config + '.ans';
  const configName = config + '.json';
  const opencc = new OpenCC(configName);
  fs.readFile(inputName, 'utf-8', function (err, text) {
    if (err) return done(err);
    opencc.convert(text, function (err, converted) {
      if (err) return done(err);
      fs.readFile(outputName, 'utf-8', function (err, answer) {
        if (err) return done(err);
        assert.equal(converted, answer);
        done();
      });
    });
  });
};

async function testAsyncPromise(config) {
  const inputName = 'test/testcases/' + config + '.in';
  const outputName = 'test/testcases/' + config + '.ans';
  const configName = config + '.json';
  const opencc = new OpenCC(configName);

  const text = await util.promisify(fs.readFile)(inputName, 'utf-8');
  const converted = await opencc.convertPromise(text);
  const answer = await util.promisify(fs.readFile)(outputName, 'utf-8');

  assert.equal(converted, answer);
};

describe('Sync API', function () {
  configs.forEach(function (config) {
    it(config, function (done) {
      testSync(config, done);
    });
  });
});

describe('Async API', function () {
  configs.forEach(function (config) {
    it(config, function (done) {
      testAsync(config, done);
    });
  });
});

describe('Async Promise API', function () {
  configs.forEach(function (config) {
    it(config, function (done) {
      testAsyncPromise(config).then(done);
    });
  });
});
