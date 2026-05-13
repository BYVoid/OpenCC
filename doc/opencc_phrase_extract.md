# opencc_phrase_extract

`opencc_phrase_extract` extracts likely word or phrase candidates from UTF-8
text corpora. It is a developer tool for finding possible dictionary entries;
it does not convert text and it does not build `.ocd2` dictionaries.

The tool reads one or more input files, concatenates their contents, runs
OpenCC's phrase extraction algorithm, and writes scored candidates to an output
file.

## Usage

```sh
opencc_phrase_extract INPUT... -o OUTPUT
```

Examples:

```sh
opencc_phrase_extract corpus.txt -o phrases.txt
opencc_phrase_extract news-1.txt news-2.txt news-3.txt -o phrases.txt
```

Arguments:

- `INPUT...`: one or more UTF-8 text files.
- `-o, --output OUTPUT`: required output file path.

Run `opencc_phrase_extract --help` to see the command-line parser help for the
installed binary.

## Output Format

Each output line contains one extracted candidate and its scores:

```text
phrase frequency log_probability cohesion entropy prefix_entropy suffix_entropy
```

Columns:

- `phrase`: extracted UTF-8 word or phrase candidate.
- `frequency`: number of occurrences counted by the extractor.
- `log_probability`: natural log of `frequency / total_occurrences`.
- `cohesion`: minimum pointwise mutual information across possible internal
  splits of the candidate. Higher values usually indicate that the characters
  bind together more strongly.
- `entropy`: `prefix_entropy + suffix_entropy`.
- `prefix_entropy`: entropy of the candidate's left context.
- `suffix_entropy`: entropy of the candidate's right context.

Higher cohesion and richer left/right entropy generally make a candidate more
likely to be a useful phrase. The output is intended for review, not direct
inclusion in OpenCC dictionaries.

## Extraction Behavior

The command-line tool currently uses the following fixed settings:

- candidate maximum length: 2 UTF-8 code points
- prefix context length: 1 UTF-8 code point
- suffix context length: 1 UTF-8 code point

Candidates containing common punctuation are filtered out. The final selection
uses thresholds in `PhraseExtract::DefaultPostCalculationFilter`; these
thresholds are heuristic and may change with the implementation.

For custom lengths, filters, or thresholds, use the `PhraseExtract` C++ class
directly instead of the command-line executable.

## Dictionary Workflow

`opencc_phrase_extract` only discovers candidate phrases. A typical dictionary
maintenance workflow is:

1. Run `opencc_phrase_extract` on a relevant corpus.
2. Review the scored output manually.
3. Add accepted entries to the appropriate text dictionary under
   `data/dictionary/`.
4. Use `opencc_dict` or the normal build to generate `.ocd2` dictionaries.
5. Run tests and conversion checks for the affected configuration.

## Notes

- Input files should be UTF-8 encoded.
- The tool reads all input contents into memory before extraction.
- Multiple input files are concatenated in the order provided.
- Existing output files are overwritten.
- Empty or unreadable input files may produce no useful candidates; verify file
  paths before running large jobs.
