var assert = require('assert');
var fs = require('fs');
var OpenCC = require('./opencc');

var configs = [
  'zhs2zht',
  'zht2zhs',
  'mix2zht',
  'mix2zhs',
  'zhs2zhtw_p',
  'zhs2zhtw_vp',
  'zhtw2zhcn_t',
  'zhtw2zhcn_s',
];

var testSync = function (config, done) {
  var inputName = 'test/testcases/' + config + '.in';
  var outputName = 'test/testcases/' + config + '.ans';
  var configName = config + '.ini';
  var opencc = new OpenCC(configName);
  fs.readFile(inputName, 'utf-8', function (err, text) {
    if (err) return done(err);
    var converted = opencc.convertSync(text);
    fs.readFile(outputName, 'utf-8', function (err, answer) {
      if (err) return done(err);
      assert.equal(converted, answer);
      done();
    });
  });
};

var testAsync = function (config, done) {
  var inputName = 'test/testcases/' + config + '.in';
  var outputName = 'test/testcases/' + config + '.ans';
  var configName = config + '.ini';
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
