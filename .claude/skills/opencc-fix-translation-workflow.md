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
    - For s2twp and tw2sp: `TWPhrases.txt`

2.  **Add the mapping**:
    Map the vocabulary to itself to prevent incorrect segmentation or conversion.
    ```text
    方程式	方程式
    ```
    *Note*: Maintain dictionary alphabetical sorting (if applicable).

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
    Run `make test` or `ctest`.


## 5. Commit

When committing, it is recommended to clearly separate or combine, but must include:
- Dictionary text file changes (`.txt`)
- Core test changes (`test/testcases/testcases.json`)

```bash
git add data/dictionary/TWPhrases.txt test/testcases/testcases.json
git commit -m "Fix(Dictionary): correct conversion for 'XYZ'"
```
