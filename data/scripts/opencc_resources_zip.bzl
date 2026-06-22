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

def _opencc_resources_ocd2_zip_impl(ctx):
    args = ctx.actions.args()
    args.add("--input", ctx.file.src)
    args.add("--output", ctx.outputs.out)
    args.add("--opencc-dict", ctx.executable.opencc_dict)

    ctx.actions.run(
        executable = ctx.executable.tool,
        arguments = [args],
        inputs = [ctx.file.src],
        tools = [ctx.executable.opencc_dict],
        outputs = [ctx.outputs.out],
        mnemonic = "OpenccResourcesOcd2Zip",
        progress_message = "Converting OpenCC resource zip to ocd2 %{output}",
    )

opencc_resources_ocd2_zip = rule(
    implementation = _opencc_resources_ocd2_zip_impl,
    attrs = {
        "src": attr.label(allow_single_file = True, mandatory = True),
        "opencc_dict": attr.label(
            default = Label("//src/tools:dict_converter"),
            executable = True,
            cfg = "exec",
        ),
        "tool": attr.label(
            default = Label("//scripts:convert_resource_zip"),
            executable = True,
            cfg = "exec",
        ),
        "out": attr.output(mandatory = True),
    },
)

