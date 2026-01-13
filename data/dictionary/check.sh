#!/usr/bin/env bash
# Usage: ./dedup.sh path/to/file
# The file is overwritten with only the first occurrence of each line.

file="${1:?need a file path}"

# Create a temporary file in the same directory
tmp="$(mktemp "${file}.XXXXXX")"

awk '
{
    if (!seen[$0]++)          # print only the first time we see the line
        print > "'"$tmp"'"
}
' "$file"

# Replace the original file with the deduped version
mv "$tmp" "$file"
