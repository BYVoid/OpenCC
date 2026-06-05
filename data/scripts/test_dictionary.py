#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import glob
import os
import unittest
from functools import cached_property

from common import Table, SmpTable

root_dir = os.path.normpath(os.path.join(__file__, "..", ".."))
dict_dir = os.path.join(root_dir, "dictionary")
scheme_dir = os.path.join(root_dir, "scheme")

ALLOWED_SMP_CHARACTERS = (
    ("STPhrases", "𫖮"),  # U+2B5AE
    ("STPhrases", "𫗧"),  # U+2B5E7
    ("STPhrases", "𫛭"),  # U+2B6ED
    ("STPhrases", "𬴃"),  # U+2CD03
    ("TSPhrases", "𫫇"),  # U+2BAC7
)


class TestDictionaries(unittest.TestCase):
    def test_compatibility_ideographs(self):
        """
        Validate that regular dictionaries do not contain Unicode CJK
        Compatibility Ideographs.
        """
        excluded_dicts = (
            "CJK_Compatibility_Ideographs",
            "TSCharactersBase",
            "TSCharactersExt",
            "TSPhrasesBase",
            "TSPhrasesExt",
        )
        comp_file = os.path.join(dict_dir, "CJK_Compatibility_Ideographs.txt")

        comp_map = {}
        for entry in Table().iter(comp_file):
            if entry:
                comp_map[entry.key] = entry[0]
        self.assertEqual(len(comp_map), 1002)

        for file in glob.iglob(os.path.join(glob.escape(dict_dir), "*.txt")):
            basename, _ = os.path.splitext(os.path.basename(file))
            if basename in excluded_dicts:
                continue

            with self.subTest(name=basename):
                for i, line in enumerate(Table()._iter(file)):
                    for char in line:
                        cp = ord(char)
                        if not ((0xF900 <= cp <= 0xFAFF) or (0x2F800 <= cp <= 0x2FA1F)):
                            continue

                        message = (
                            f"{basename}:{i + 1} contains CJK Compatibility Ideograph U+{cp:04X}. "
                        )

                        if char in comp_map:
                            rep_char = comp_map[char]
                            message += f"Replace it with {rep_char} U+{ord(rep_char):04X}."
                        else:
                            message += (
                                "No UnicodeData decomposition mapping is available; "
                                "replace it manually with the standard CJK unified ideograph form."
                            )

                        self.fail(message)

    def test_non_bmp(self):
        """
        Validate non-BMP characters in phrase and variant dictionaries.
        """
        excluded_dicts = (
            "CJK_Compatibility_Ideographs",
            "TSCharacters",
            "TSCharactersBase",
            "TSCharactersExt",
            "TSPhrasesBase",
            "TSPhrasesExt",
            "STCharacters",
        )

        for file in glob.iglob(os.path.join(glob.escape(dict_dir), "*.txt")):
            basename, _ = os.path.splitext(os.path.basename(file))
            if basename in excluded_dicts:
                continue

            with self.subTest(name=basename):
                for i, line in enumerate(Table()._iter(file)):
                    for char in line:
                        if (
                            ord(char) > 0xFFFF and
                            not any(basename == f and char == c for f, c in ALLOWED_SMP_CHARACTERS)
                        ):
                            self.fail(
                                f"{basename}:{i + 1} contains non-BMP character {char!r}. "
                                f"Add it to `ALLOWED_SMP_CHARACTERS` only if it is intentional."
                            )

    def test_phrase_character_dependency(self):
        """
        Validate that phrase-level character substitutions remain declared in the
        corresponding character dictionary.
        """
        dict_pairs = (
            ("STPhrases", "STCharacters"),
            ("TSPhrases", "TSCharacters"),
            ("HKVariantsPhrases", "HKVariants"),
            ("HKVariantsRevPhrases", "HKVariantsRev"),
            ("TWVariantsPhrases", "TWVariants"),
            ("TWVariantsRevPhrases", "TWVariantsRev"),
        )

        for phrase_name, char_name in dict_pairs:
            with self.subTest(phrase=phrase_name, character=char_name):
                phrase_file = os.path.join(dict_dir, f"{phrase_name}.txt")
                char_file = os.path.join(dict_dir, f"{char_name}.txt")

                # Special handling: load swapped dict from "<file>" if "<file>Rev" not generated
                if not os.path.isfile(char_file) and char_name.endswith("Rev"):
                    char_file = os.path.join(dict_dir, f"{char_name[:-3]}.txt")
                    table = Table.from_file(char_file)
                    table.swap()
                    entries = table.values()
                else:
                    entries = Table().iter(char_file)

                char_cands = {}
                for entry in entries:
                    key = entry.key
                    values = entry
                    if not (len(key) == 1 and values):
                        continue
                    char_cands[key] = set(values)

                failures = []
                for entry in Table().iter(phrase_file):
                    key = entry.key
                    for value in entry:
                        if len(key) != len(value):
                            continue

                        for k_char, v_char in zip(key, value):
                            if k_char == v_char:
                                continue

                            if v_char in char_cands.get(k_char, set()):
                                continue

                            failures.append(
                                f"{phrase_name}:{entry.line} maps {key!r} -> {value!r}, "
                                f"including character substitution {k_char!r} -> {v_char!r}, "
                                f"but {char_name} does not list that target as a candidate."
                            )

                if failures:
                    self.fail(
                        "Phrase-level character substitutions must also be declared in the "
                        "corresponding character dictionary. If a direct character conversion "
                        "is not generally desirable, keep it as a non-default candidate such "
                        "as A<TAB>A B instead of deleting it outright:\n" + "\n".join(failures)
                    )

    def test_variant_rev_phrases(self):
        """Validate reverse variant phrase dictionary coverage."""
        dict_names = (
            "HKVariants",
            "TWVariants",
        )
        st_characters_file = os.path.join(dict_dir, "STCharacters.txt")
        st_phrases_file = os.path.join(dict_dir, "STPhrases.txt")

        for dict_name in dict_names:
            with self.subTest(name=dict_name):
                phrases_file_name = f"{dict_name}RevPhrases"

                variants_file = os.path.join(dict_dir, f"{dict_name}.txt")
                phrases_file = os.path.join(dict_dir, f"{phrases_file_name}.txt")

                variant_sets = [e for e in Table().iter(st_characters_file) if len(e) > 1]

                exception_chars = set()
                for entry in Table().iter(variants_file):
                    key = entry.key
                    for v in entry:
                        if key != v and any(key in v_set and v in v_set for v_set in variant_sets):
                            exception_chars.add(v)

                expected_phrases = set()
                for values in Table().iter(st_phrases_file):
                    if values and len(values[0]) >= 2 and any(c in exception_chars for c in values[0]):
                        expected_phrases.add(values[0])

                identity_keys = {
                    entry.key for entry in Table().iter(phrases_file)
                    if entry.key in entry
                }

                missing_phrases = [p for p in expected_phrases if p not in identity_keys]
                if missing_phrases:
                    self.fail(
                        f"{phrases_file_name} missing reverse phrase identity exceptions:\n" +
                        "\n".join(missing_phrases)
                    )

    def test_phrases_reverse_mapping(self):
        """
        Validate that regional phrase and reversed phrase dictionaries match
        each other.
        """
        dict_names = (
            "HKPhrases",
            "TWPhrases",
        )

        for dict_name in dict_names:
            with self.subTest(name=dict_name):
                dict_name_rev = f"{dict_name}Rev"

                table = Table.from_file(os.path.join(dict_dir, f"{dict_name}.txt"))
                table_rev = Table.from_file(os.path.join(dict_dir, f"{dict_name_rev}.txt"))

                for key, values in table.items():
                    for value in values:
                        if key not in table_rev.get(value, []):
                            self.fail(f"Missing reverse mapping in {dict_name_rev}: {key} -> {value}")

                for key, values in table_rev.items():
                    for value in values:
                        if key not in table.get(value, []):
                            self.fail(f"Missing reverse mapping in {dict_name}: {key} -> {value}")

    def test_phrases_segmentation(self):
        """
        Validate that regional phrase keys are not interrupted by s2t phrase
        values during a phrase-related conversion like s2twp.
        """
        dict_names = (
            "HKPhrases",
            "TWPhrases",
        )
        st_phrases_file = os.path.join(dict_dir, "STPhrases.txt")

        st_values = {}
        for entry in Table().iter(st_phrases_file):
            for value in entry:
                if value not in st_values:
                    st_values[value] = {
                        "key": entry.key,
                        "value": value,
                        "line": entry.line,
                    }

        for dict_name in dict_names:
            with self.subTest(name=dict_name):
                phrases_file = os.path.join(dict_dir, f"{dict_name}.txt")

                missing = []
                for entry in Table().iter(phrases_file):
                    key = entry.key
                    if any(key in value for value in st_values):
                        continue

                    for st_entry in st_values.values():
                        value = st_entry["value"]
                        if len(value) >= len(key) or len(value) < 2:
                            continue

                        pos = key.find(value)
                        if pos == -1:
                            continue

                        if pos == 0:
                            category = "prefix"
                        elif pos + len(value) == len(key):
                            category = "suffix"
                        else:
                            category = "middle"

                        missing.append(
                            f"{dict_name!r} key {key!r} is not covered by any STPhrases value.\n"
                            f"  Conflicting STPhrases record: STPhrases.txt:{st_entry['line']} {st_entry['key']!r} -> {value!r}\n"
                            f"  The existing value appears as a {category} fragment of the {dict_name!r} key."
                        )
                        break

                if missing:
                    self.fail(
                        f"Potential missing STPhrases entries for {dict_name!r} segmentation:\n" +
                        "\n".join(missing)
                    )


if __name__ == "__main__":
    unittest.main()
