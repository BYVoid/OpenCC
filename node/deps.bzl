"""Module extension to fetch Node.js native addon build dependencies.

Fetches:
  - node-addon-api 8.7.0  → @node_addon_api
  - Node.js 20.20.2 headers → @node_headers  (provides node_api.h etc.)
  - Node.js 20.20.2 win-x64 node.lib → @node_win_x64_lib
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

def _node_deps_impl(ctx):
    # node-addon-api 8.7.0
    http_archive(
        name = "node_addon_api",
        url = "https://registry.npmjs.org/node-addon-api/-/node-addon-api-8.7.0.tgz",
        sha256 = "06cdc368599c65b996003ac5d71fe594a78d3d94fc51600b2085d5a325a3d930",
        strip_prefix = "package",
        build_file = Label("//node:node_addon_api.BUILD"),
    )

    # Node.js 20.20.2 headers (node_api.h, v8.h, etc.)
    http_archive(
        name = "node_headers",
        url = "https://nodejs.org/dist/v20.20.2/node-v20.20.2-headers.tar.gz",
        sha256 = "6de0e836efa9f32512e61db3dfd08b3d97a015b7e828d1a5efdf281a56a692d9",
        strip_prefix = "node-v20.20.2",
        build_file = Label("//node:node_headers.BUILD"),
    )

    # Node.js import library for Windows addon builds.
    http_file(
        name = "node_win_x64_lib",
        url = "https://nodejs.org/dist/v20.20.2/win-x64/node.lib",
        sha256 = "c4a794e993d9304238523230885e9ec00ca052c73b9558471858eef14916d91f",
    )

node_deps = module_extension(
    implementation = _node_deps_impl,
)
