// Entry point for the Bazel js_test (//node/test:node_test): run node/test.js
// with Node's built-in test runner (node:test) against the self-contained
// sandbox assembled next to this file. The test has no npm dependencies at all:
// node:test ships with Node, and node/opencc.js locates the addon with a plain
// filesystem lookup (no node-gyp-build).
//
// The sandbox mirrors an npm-install layout (node/*.js + build/Release/<addon> +
// assets + test/testcases + scripts), so node/opencc.js resolves the addon and
// its resources from its own location, exactly as it does after `npm install`.
const path = require('path');

// Staged at <runfiles>/_main/node/test/test_entry.js.
const sandbox = path.join(__dirname, 'sandbox');
process.chdir(sandbox);

// node/test.js registers tests via node:test; requiring it lets the built-in
// runner execute them and set a non-zero exit code on failure.
require(path.join(sandbox, 'node', 'test.js'));
