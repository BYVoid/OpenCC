#!/usr/bin/env python3

import argparse
import difflib
import json
import zipfile
from pathlib import Path


MANIFEST_NAME = "opencc-resource-manifest.json"
MAX_DIFF_LINES_PER_FILE = 400
MAX_EXAMPLES = 20


def parse_args():
    parser = argparse.ArgumentParser(
        description="Compare two OpenCC resource zip archives."
    )
    parser.add_argument("before")
    parser.add_argument("after")
    parser.add_argument("--markdown", required=True)
    return parser.parse_args()


def read_zip(path):
    entries = {}
    with zipfile.ZipFile(path) as archive:
        for info in archive.infolist():
            if info.is_dir():
                continue
            if info.compress_type != zipfile.ZIP_STORED:
                raise ValueError(
                    f"{path}: {info.filename} uses unsupported compression "
                    f"method {info.compress_type}; expected ZIP_STORED"
                )
            entries[info.filename] = {
                "data": archive.read(info.filename),
                "size": info.file_size,
            }
    return entries


def decode_text(name, data):
    try:
        return data.decode("utf-8")
    except UnicodeDecodeError:
        raise ValueError(f"{name}: resource is not valid UTF-8")


def load_manifest(entries):
    entry = entries.get(MANIFEST_NAME)
    if not entry:
        return {}
    try:
        return json.loads(decode_text(MANIFEST_NAME, entry["data"]))
    except json.JSONDecodeError as error:
        raise ValueError(f"{MANIFEST_NAME}: invalid JSON: {error}") from error


def parse_dict(text):
    result = {}
    for line in text.splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        if "\t" not in line:
            continue
        key, value = line.split("\t", 1)
        result[key] = value
    return result


def dictionary_summary(before_text, after_text):
    before = parse_dict(before_text)
    after = parse_dict(after_text)
    before_keys = set(before)
    after_keys = set(after)
    added = sorted(after_keys - before_keys)
    removed = sorted(before_keys - after_keys)
    changed = sorted(key for key in before_keys & after_keys if before[key] != after[key])
    return {
        "added": added,
        "removed": removed,
        "changed": changed,
        "before": before,
        "after": after,
    }


def append_examples(lines, title, keys, values):
    if not keys:
        return
    lines.append(f"- {title}: {len(keys)}")
    for key in keys[:MAX_EXAMPLES]:
        lines.append(f"  - `{key}` -> `{values[key]}`")
    if len(keys) > MAX_EXAMPLES:
        lines.append(f"  - ... {len(keys) - MAX_EXAMPLES} more")


def dictionary_file_summary(name, data):
    if not name.endswith(".txt"):
        return None
    text = decode_text(name, data)
    entries = parse_dict(text)
    return entries


def append_dictionary_file_examples(lines, entries):
    if entries is None:
        return
    for key in sorted(entries)[:MAX_EXAMPLES]:
        lines.append(f"  - `{key}` -> `{entries[key]}`")
    if len(entries) > MAX_EXAMPLES:
        lines.append(f"  - ... {len(entries) - MAX_EXAMPLES} more")


def append_text_diff(lines, name, before_text, after_text):
    diff = list(
        difflib.unified_diff(
            before_text.splitlines(),
            after_text.splitlines(),
            fromfile=f"before/{name}",
            tofile=f"after/{name}",
            lineterm="",
        )
    )
    if not diff:
        return
    truncated = len(diff) > MAX_DIFF_LINES_PER_FILE
    if truncated:
        diff = diff[:MAX_DIFF_LINES_PER_FILE]
    lines.append("<details>")
    lines.append(f"<summary>Unified diff for <code>{name}</code></summary>")
    lines.append("")
    lines.append("```diff")
    lines.extend(diff)
    if truncated:
        lines.append(
            f"... truncated after {MAX_DIFF_LINES_PER_FILE} lines; inspect the zip "
            "artifacts for the full diff."
        )
    lines.append("```")
    lines.append("")
    lines.append("</details>")
    lines.append("")


def metadata_line(label, manifest):
    if not manifest:
        return f"- {label}: manifest missing"
    source = manifest.get("source_url", "unknown")
    commit = manifest.get("commit_id", "unknown")
    dirty = manifest.get("source_dirty", "unknown")
    build_time = manifest.get("build_time_utc", "unknown")
    return (
        f"- {label}: `{commit}`"
        f" (source: `{source}`, dirty: `{dirty}`, built: `{build_time}`)"
    )


def write_report(before_path, after_path, before_entries, after_entries, output_path):
    before_manifest = load_manifest(before_entries)
    after_manifest = load_manifest(after_entries)
    before_names = set(before_entries)
    after_names = set(after_entries)
    compare_names = sorted((before_names | after_names) - {MANIFEST_NAME})
    added_files = sorted(after_names - before_names - {MANIFEST_NAME})
    removed_files = sorted(before_names - after_names - {MANIFEST_NAME})
    changed_files = [
        name
        for name in compare_names
        if name in before_entries
        and name in after_entries
        and before_entries[name]["data"] != after_entries[name]["data"]
    ]

    lines = [
        "# OpenCC Resource Zip Diff",
        "",
        "## Inputs",
        "",
        f"- Before zip: `{before_path}`",
        f"- After zip: `{after_path}`",
        metadata_line("Before manifest", before_manifest),
        metadata_line("After manifest", after_manifest),
        "",
        "## Summary",
        "",
        f"- Added files: {len(added_files)}",
        f"- Removed files: {len(removed_files)}",
        f"- Changed files: {len(changed_files)}",
        f"- Unchanged files: {len(compare_names) - len(added_files) - len(removed_files) - len(changed_files)}",
        "",
    ]

    if added_files:
        lines.append("## Added Files")
        lines.append("")
        for name in added_files:
            entries = dictionary_file_summary(name, after_entries[name]["data"])
            entry_suffix = f", {len(entries)} entries" if entries is not None else ""
            lines.append(f"- `{name}` ({after_entries[name]['size']} bytes{entry_suffix})")
            append_dictionary_file_examples(lines, entries)
        lines.append("")
    if removed_files:
        lines.append("## Removed Files")
        lines.append("")
        for name in removed_files:
            entries = dictionary_file_summary(name, before_entries[name]["data"])
            entry_suffix = f", {len(entries)} entries" if entries is not None else ""
            lines.append(
                f"- `{name}` ({before_entries[name]['size']} bytes{entry_suffix})"
            )
            append_dictionary_file_examples(lines, entries)
        lines.append("")

    if changed_files:
        lines.append("## Changed Files")
        lines.append("")
        for name in changed_files:
            before_text = decode_text(name, before_entries[name]["data"])
            after_text = decode_text(name, after_entries[name]["data"])
            lines.append(
                f"### `{name}` ({before_entries[name]['size']} -> {after_entries[name]['size']} bytes)"
            )
            lines.append("")
            if name.endswith(".txt"):
                summary = dictionary_summary(before_text, after_text)
                append_examples(lines, "Added entries", summary["added"], summary["after"])
                append_examples(lines, "Removed entries", summary["removed"], summary["before"])
                changed_values = {
                    key: f"{summary['before'][key]} -> {summary['after'][key]}"
                    for key in summary["changed"]
                }
                append_examples(lines, "Changed entries", summary["changed"], changed_values)
                lines.append("")
            append_text_diff(lines, name, before_text, after_text)

    if before_manifest or after_manifest:
        lines.append("## Manifest Entry Hash Changes")
        lines.append("")
        before_hashes = before_manifest.get("entries", {})
        after_hashes = after_manifest.get("entries", {})
        hash_changed = [
            name
            for name in sorted(set(before_hashes) | set(after_hashes))
            if before_hashes.get(name, {}).get("sha256")
            != after_hashes.get(name, {}).get("sha256")
        ]
        if hash_changed:
            lines.extend(f"- `{name}`" for name in hash_changed)
        else:
            lines.append("- No manifest entry hash changes.")
        lines.append("")

    if not added_files and not removed_files and not changed_files:
        lines.append("No resource content changes.")
        lines.append("")

    Path(output_path).write_text("\n".join(lines), encoding="utf-8")


def main():
    args = parse_args()
    before_entries = read_zip(args.before)
    after_entries = read_zip(args.after)
    write_report(args.before, args.after, before_entries, after_entries, args.markdown)


if __name__ == "__main__":
    main()
