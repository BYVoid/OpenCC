#!/usr/bin/env python3

import argparse
import datetime
import json
import re
import sys
from pathlib import Path
from typing import Any, Dict, List

def get_opencc_version() -> str:
    try:
        script_dir = Path(__file__).resolve().parent
        repo_root = script_dir.parent
        package_json_path = repo_root / "package.json"
        if package_json_path.exists():
            with package_json_path.open("r", encoding="utf-8") as f:
                data = json.load(f)
                return data.get("version", "unknown")
    except Exception:
        pass
    return "unknown"

def parse_jsonc(text: str) -> Any:
    # remove single line comments
    text = re.sub(r'//.*', '', text)
    # remove multi line comments
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)
    # remove trailing commas
    text = re.sub(r',\s*([\]}])', r'\1', text)
    return json.loads(text)

def parse_txt_dict(path: Path) -> Dict[str, List[str]]:
    entries: Dict[str, List[str]] = {}
    with path.open("r", encoding="utf-8") as f:
        for line in f:
            if line.startswith("#") or not line.strip():
                continue
            parts = line.rstrip("\r\n").split("\t", 1)
            if len(parts) < 2:
                continue
            key = parts[0]
            values = parts[1].split(" ")
            entries[key] = values
    return entries

def reverse_dict_entries(entries: Dict[str, List[str]]) -> Dict[str, List[str]]:
    reversed_entries: Dict[str, List[str]] = {}
    for key, values in entries.items():
        for val in values:
            if val not in reversed_entries:
                reversed_entries[val] = []
            reversed_entries[val].append(key)
    return reversed_entries

def load_dict(name: str, dict_dir: Path) -> Dict[str, List[str]]:
    # Check if it is TSCharactersExt
    if name == "TSCharactersExt":
        return {}  # Ignore as requested
        
    # Try to load name.txt
    txt_path = dict_dir / f"{name}.txt"
    if txt_path.exists():
        return parse_txt_dict(txt_path)
    
    # Check if it is a reversed dictionary
    if name.endswith("Rev"):
        base_name = name[:-3]
        base_txt_path = dict_dir / f"{base_name}.txt"
        if base_txt_path.exists():
            base_entries = parse_txt_dict(base_txt_path)
            return reverse_dict_entries(base_entries)
            
    raise FileNotFoundError(f"Dictionary file not found for: {name} in {dict_dir}")

def inline_entries_for_file_dict(dict_def: Dict[str, Any], dict_dir: Path) -> Dict[str, str]:
    file_name = dict_def["file"]
    base_name = Path(file_name).stem
    raw_dict = load_dict(base_name, dict_dir)
    return {k: v[0] for k, v in raw_dict.items()}

def compile_dict(dict_def: Dict[str, Any], dict_dir: Path) -> Dict[str, Any]:
    if dict_def["type"] == "group":
        result: Dict[str, Any] = {"type": "group"}
        if "match_policy" in dict_def:
            result["match_policy"] = dict_def["match_policy"]
        result["dicts"] = [compile_dict(sub_def, dict_dir) for sub_def in dict_def["dicts"]]
        return result

    if dict_def["type"] == "inline":
        entries = dict_def["entries"]
    else:
        entries = inline_entries_for_file_dict(dict_def, dict_dir)

    return {
        "type": "inline",
        "entries": sort_dict_keys(entries),
    }

def sort_dict_keys(d: Dict[str, str]) -> Dict[str, str]:
    return {k: d[k] for k in sorted(d.keys())}

def compile_config(config_path: Path, dict_dir: Path) -> Dict[str, Any]:
    with config_path.open("r", encoding="utf-8") as f:
        config_content = f.read()
    
    config_data = parse_jsonc(config_content)
    
    # Process segmentation dict if present
    if "segmentation" in config_data and "dict" in config_data["segmentation"]:
        dict_def = config_data["segmentation"]["dict"]
        config_data["segmentation"]["dict"] = compile_dict(dict_def, dict_dir)
        
    # Process conversion chain dicts
    if "conversion_chain" in config_data:
        for stage in config_data["conversion_chain"]:
            if "dict" in stage:
                dict_def = stage["dict"]
                stage["dict"] = compile_dict(dict_def, dict_dir)
                
    return config_data

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Compile OpenCC config and dicts to a single inline JSON config.",
        epilog="Example:\n"
               "  python3 scripts/compile_to_inline_config.py -c data/config/s2t.json -d data/dictionary -o tmp/s2t_inline.json\n",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("-c", "--config", required=True, type=Path, help="Path to original OpenCC JSON config")
    parser.add_argument("-d", "--dict-dir", required=True, type=Path, help="Directory containing .txt source dictionaries")
    parser.add_argument("-o", "--output", required=True, type=Path, help="Output path for inlined JSON config")
    
    args = parser.parse_args()
    
    if not args.config.exists():
        print(f"Error: Config file not found: {args.config}", file=sys.stderr)
        return 1
    if not args.dict_dir.is_dir():
        print(f"Error: Dictionary directory not found: {args.dict_dir}", file=sys.stderr)
        return 1
        
    try:
        compiled_config = compile_config(args.config, args.dict_dir)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        
        version = get_opencc_version()
        timestamp = datetime.datetime.now().astimezone().strftime('%Y-%m-%d %H:%M:%S %Z')
        metadata_comments = (
            "// Generated by OpenCC compile_to_inline_config.py\n"
            f"// OpenCC Version: {version}\n"
            f"// Source Config: {args.config.name}\n"
            f"// Compiled At: {timestamp}\n\n"
        )
        
        with args.output.open("w", encoding="utf-8") as f:
            f.write(metadata_comments)
            json.dump(compiled_config, f, ensure_ascii=False, indent=2)
            f.write("\n")
        print(f"Successfully compiled to {args.output}")
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

if __name__ == "__main__":
    raise SystemExit(main())
