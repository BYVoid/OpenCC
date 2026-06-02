"""BUILD file for Node.js 20 headers (node_api.h, v8.h, etc.)."""

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "node_headers",
    hdrs = glob(["include/node/**/*.h"]),
    includes = ["include/node"],
    strip_include_prefix = "include/node",
)

filegroup(
    name = "all_files",
    srcs = glob(["include/node/**/*.h"]),
)

filegroup(
    name = "node_api_header",
    srcs = ["include/node/node_api.h"],
)
