var assert = require('assert');
var fs = require('fs');
var OpenCC = require('./opencc');

var configs = [
  's2t',
  's2tw',
  's2twp',
  't2s',
  'tw2s',
  'tw2sp',
  's2hk',
  'hk2s',
];

var testSync = function (config, done) {
  var inputName = 'test/testcases/' + config + '.in';
  var outputName = 'test/testcases/' + config + '.ans';
  var configName = config + '.json';
  var opencc = new OpenCC(configName);
  var text = fs.readFileSync(inputName, 'utf-8');
  var converted = opencc.convertSync(text);
  var answer = fs.readFileSync(outputName, 'utf-8');
  assert.equal(converted, answer);
  done();
};

var testAsync = function (config, done) {
  var inputName = 'test/testcases/' + config + '.in';
  var outputName = 'test/testcases/' + config + '.ans';
  var configName = config + '.json';
  var opencc = new OpenCC(configName);
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
