"""BUILD file for node-addon-api (napi.h, napi-inl.h, napi-inl.deprecated.h)."""

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "node_addon_api",
    hdrs = glob(["*.h"]),
    includes = ["."],
    defines = ["NAPI_DISABLE_CPP_EXCEPTIONS"],
)

filegroup(
    name = "all_files",
    srcs = glob(["*.h"]),
)

filegroup(
    name = "napi_header",
    srcs = ["napi.h"],
)
