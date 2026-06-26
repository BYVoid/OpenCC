// Entry point for the Bazel js_test (//node/test:node_test): run node/test.js
// through Mocha's programmatic API against the self-contained sandbox assembled
// next to this file.
//
// The sandbox mirrors an npm-install layout (node/*.js + build/Release/<addon> +
// assets + test/testcases + scripts), so node/opencc.js resolves the addon and
// its resources from its own location, exactly as it does after `npm install`.
const path = require('path');
const Mocha = require('mocha');

// Staged at <runfiles>/_main/node/test/mocha_entry.js.
const sandbox = path.join(__dirname, 'sandbox');
process.chdir(sandbox);

const mocha = new Mocha({ reporter: 'spec' });
mocha.addFile(path.join(sandbox, 'node', 'test.js'));
mocha.run((failures) => {
  process.exitCode = failures ? 1 : 0;
});
