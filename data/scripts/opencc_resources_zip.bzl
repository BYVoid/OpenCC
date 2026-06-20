def _opencc_resources_zip_impl(ctx):
    args = ctx.actions.args()
    args.add("--output", ctx.outputs.out)
    args.add("--source-url", ctx.attr.source_url)
    args.add("--stable-status", ctx.info_file)
    args.add("--volatile-status", ctx.version_file)
    args.add_all("--configs", ctx.files.configs)
    args.add_all("--dicts", ctx.files.dicts)

    ctx.actions.run(
        executable = ctx.executable.tool,
        arguments = [args],
        inputs = (
            ctx.files.configs +
            ctx.files.dicts +
            [ctx.info_file, ctx.version_file]
        ),
        outputs = [ctx.outputs.out],
        mnemonic = "OpenccResourcesZip",
        progress_message = "Building OpenCC resource zip %{output}",
    )

opencc_resources_zip = rule(
    implementation = _opencc_resources_zip_impl,
    attrs = {
        "configs": attr.label_list(allow_files = True, mandatory = True),
        "dicts": attr.label_list(allow_files = True, mandatory = True),
        "source_url": attr.string(default = "https://github.com/BYVoid/OpenCC"),
        "tool": attr.label(
            default = Label("//data/scripts:opencc_resources_zip"),
            executable = True,
            cfg = "exec",
        ),
        "out": attr.output(mandatory = True),
    },
)
