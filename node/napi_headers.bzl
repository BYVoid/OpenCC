"""Expose node-addon-api's header files as a plain file list.

The BCR `@node_addon_api//:node_addon_api` target is a header-only `cc_library`,
so it only carries its headers inside `CcInfo`. The Windows-Zig genrule does not
go through the C++ toolchain; it shells out to `zig c++` and needs the header
*files* on disk (and the path of `napi.h` to derive an `-I` include dir). This
rule pulls those headers back out of `CcInfo` so the genrule can consume them.
"""

def _cc_library_headers_impl(ctx):
    headers = ctx.attr.lib[CcInfo].compilation_context.direct_public_headers
    if ctx.attr.only:
        headers = [h for h in headers if h.basename == ctx.attr.only]
    return [DefaultInfo(files = depset(headers))]

cc_library_headers = rule(
    implementation = _cc_library_headers_impl,
    doc = "Collect the public header files of a cc_library as a file list.",
    attrs = {
        "lib": attr.label(
            mandatory = True,
            providers = [CcInfo],
            doc = "The cc_library whose headers should be collected.",
        ),
        "only": attr.string(
            default = "",
            doc = "If set, keep only the header whose basename matches this value.",
        ),
    },
)
