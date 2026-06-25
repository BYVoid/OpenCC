"""Module extension to fetch Node.js native addon build dependencies.

node-addon-api itself comes from the Bazel Central Registry (see the
`bazel_dep(name = "node_addon_api", ...)` in MODULE.bazel). This extension only
fetches the pieces that are not available there:
  - Node.js 20.20.2 headers → @node_headers  (provides node_api.h etc.)
  - Node.js 20.20.2 win-x64 node.lib → @node_win_x64_lib
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

def _node_deps_impl(ctx):
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
