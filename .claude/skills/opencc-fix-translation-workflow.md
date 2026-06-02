---
name: opencc-fix-translation-workflow
description: OpenCC translation fix and complete release workflow
tags: [opencc, workflow, debugging]
---

# OpenCC Translation Fix Standard Operating Procedure

This skill describes the complete lifecycle for fixing OpenCC conversion errors (such as "方程式" becoming "方程序"), including core dictionary correction, testing, and verification.

## 1. Problem Diagnosis

When a conversion error is discovered (e.g., A is incorrectly converted to B):

1.  **Search for existing mappings**:
    Use `grep` to search for the error source in `data/dictionary`.
    ```bash
    grep "error_term" data/dictionary/*.txt
    ```
2.  **Identify the interference source**:
    Usually because in Maximum Forward Matching (MaxMatch), a "longer word" contains the target word, or a "shorter word" mapping causes the incorrect result.
    *Example*: "方程式" is incorrectly converted to "方程序" because the mapping "程式" → "程序" exists, and "方程式" is not defined as a proper noun, causing it to be segmented as "方" + "程式".

## 2. Fix Solution (Explicit Mapping)

If the error originates from segmentation logic (as in the example above), the most robust fix is to **add an Explicit Mapping**.

1.  **Select the correct dictionary file**:
    - For Simplified-to-Traditional base phrase conversion and segmentation: `STPhrases.txt`.
    - For Taiwan regional vocabulary / terminology choices used by `s2twp` and `tw2sp`: `TWPhrases.txt` and `TWPhrasesRev.txt`.
    - For Taiwan character-variant phrase exceptions used by `s2tw`, `s2twp`, and `t2tw`: `TWVariantsPhrases.txt`.
    - Do **not** put word-level regional vocabulary translations in `TWVariantsPhrases.txt`; that file is for phrase-level overrides to character variant conversion. Example: US state or territory names such as `特拉華 -> 德拉瓦`, `新澤西 -> 紐澤西`, and `美屬維爾京羣島 -> 美屬維京群島` belong in `TWPhrases.txt`.
    - For `s2twp`, the conversion chain is `STPhrases/STCharacters -> TWPhrases -> TWVariantsPhrases/TWVariants`. If the Simplified input must survive as one phrase until the `TWPhrases` stage, add the needed Simplified-to-Traditional entry in `STPhrases.txt` for segmentation.

2.  **Add the mapping**:
    Map the vocabulary to itself to prevent incorrect segmentation or conversion.
    ```text
    方程式	方程式
    ```
    *Note*: Maintain bidirectional consistency for `TWPhrases.txt` and `TWPhrasesRev.txt` when adding regional vocabulary mappings.

3.  **Sort edited dictionaries**:
    Use the project script instead of hand-sorting.
    ```bash
    python3 data/scripts/sort.py data/dictionary/TWPhrases.txt data/dictionary/TWPhrases.txt
    python3 data/scripts/sort.py data/dictionary/TWPhrasesRev.txt data/dictionary/TWPhrasesRev.txt
    python3 data/scripts/sort.py data/dictionary/STPhrases.txt data/dictionary/STPhrases.txt
    ```

## 3. Test-Driven (Test Cases)

Before the modification takes effect, create test cases to ensure the fix and prevent regression.

1.  **Core tests**:
    Edit `test/testcases/testcases.json`.
    ```json
    {
      "id": "case_XXX",
      "input": "方程式",
      "expected": {
        "tw2sp": "方程式"
      }
    }
    ```
    Test the config that actually uses the dictionary. `TWPhrases.txt` mappings should normally be asserted under `s2twp` / `tw2sp`, not `s2tw`.

## 4. Build and Verify

OpenCC uses the CMake/Make system to build dictionaries.

1.  **Rebuild dictionaries**:
    ```bash
    cd build/dbg  # or your build directory
    make Dictionaries
    ```
    This step regenerates the `.ocd2` binary dictionaries.

2.  **Manual verification**:
    Test directly using the generated `opencc` tool.
    ```bash
    echo "方程式" | ./src/tools/opencc -c root/share/opencc/tw2sp.json
    # Expected output: 方程式
    ```

3.  **Automated testing** (optional but recommended):
    Run focused dictionary and command-line tests. For Bazel:
    ```bash
    bazel test //data/dictionary:dictionary_TWPhrases_test \
      //data/dictionary:dictionary_TWPhrasesRev_test \
      //data/dictionary:dictionary_TWPhrases_reverse_mapping_test \
      //data/dictionary:dictionary_STPhrases_test \
      //test:command_line_converter_test
    ```
    Use the relevant subset when only some dictionaries changed.


## 5. Commit

When committing, it is recommended to clearly separate or combine, but must include:
- Dictionary text file changes (`.txt`)
- Core test changes (`test/testcases/testcases.json`)

```bash
git add data/dictionary/TWPhrases.txt test/testcases/testcases.json
git commit -m "Fix(Dictionary): correct conversion for 'XYZ'"
```
