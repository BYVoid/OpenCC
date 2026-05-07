"""BUILD file for Node.js 20 headers (node_api.h, v8.h, etc.)."""

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "node_headers",
    hdrs = glob(["include/node/**/*.h"]),
    includes = ["include/node"],
    strip_include_prefix = "include/node",
)
