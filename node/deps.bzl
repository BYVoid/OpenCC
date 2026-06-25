"""Module extension to fetch Node.js native addon build dependencies.

node-addon-api itself comes from the Bazel Central Registry (see the
`bazel_dep(name = "node_addon_api", ...)` in MODULE.bazel) and the Node C
headers come from the rules_nodejs Node toolchain. This extension only fetches
the one piece available from neither:
  - Node.js 20.20.2 win-x64 node.lib → @node_win_x64_lib
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_file")

def _node_deps_impl(ctx):
    # Node.js import library for Windows addon builds.
    http_file(
        name = "node_win_x64_lib",
        url = "https://nodejs.org/dist/v20.20.2/win-x64/node.lib",
        sha256 = "c4a794e993d9304238523230885e9ec00ca052c73b9558471858eef14916d91f",
    )

node_deps = module_extension(
    implementation = _node_deps_impl,
)
