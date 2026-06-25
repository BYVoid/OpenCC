"""Expose a cc_library's compilation headers as raw files.

SCOPE: This is used ONLY by the Windows-Zig cross-compile genrule
(//node:opencc_node_windows_zig), which shells out to `zig c++` and therefore
needs raw Node header *files* it can pass via $(location) / -I. It is NOT part
of the main native-addon build path: the regular cc_binary (//node:opencc_node)
consumes the Node headers as a normal cc_library dependency
(@rules_nodejs//nodejs/headers:current_node_cc_headers) and never goes through
these rules.

rules_nodejs only exposes the active Node toolchain's headers as a cc_library
(CcInfo), not as a filegroup of raw files, so these rules unwrap the
CcInfo.compilation_context.headers depset into DefaultInfo files.
"""

def _cc_headers_files_impl(ctx):
    # All (transitive) headers the library exposes to its consumers.
    headers = ctx.attr.lib[CcInfo].compilation_context.headers
    return [DefaultInfo(files = headers)]

cc_headers_files = rule(
    implementation = _cc_headers_files_impl,
    doc = "Collect all of a cc_library's compilation headers as raw files.",
    attrs = {
        "lib": attr.label(providers = [CcInfo]),
    },
)

def _cc_header_pick_impl(ctx):
    # $(location) requires a single-file output, so pick one header by basename.
    wanted = ctx.attr.basename
    picked = [
        h
        for h in ctx.attr.lib[CcInfo].compilation_context.headers.to_list()
        if h.basename == wanted
    ]
    if len(picked) != 1:
        fail("expected exactly one header named %s, found %d" % (wanted, len(picked)))
    return [DefaultInfo(files = depset(picked))]

cc_header_pick = rule(
    implementation = _cc_header_pick_impl,
    doc = "Pick a single header file from a cc_library by basename (for $(location)).",
    attrs = {
        "lib": attr.label(providers = [CcInfo]),
        "basename": attr.string(mandatory = True),
    },
)
