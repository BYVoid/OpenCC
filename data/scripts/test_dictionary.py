#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import glob
import os
import unittest
from functools import cached_property

from common import Table, CjkCompTable, SmpTable

root_dir = os.path.normpath(os.path.join(__file__, "..", ".."))
dict_dir = os.path.join(root_dir, "dictionary")
scheme_dir = os.path.join(root_dir, "scheme")


class TestDictionaries(unittest.TestCase):
    @cached_property
    def cjk_comp_table(cls):
        scheme_file = os.path.join(scheme_dir, "CJKCompatibilityIdeographs.txt")
        return CjkCompTable.from_file(scheme_file)

    @cached_property
    def smp_table(cls):
        scheme_file = os.path.join(scheme_dir, "AllowedSmpChars.txt")
        return SmpTable.from_file(scheme_file)

    def test_sorted(self):
        """Validate that dictionaries are sorted."""
        excluded_dicts = (
            "TSCharactersBase.txt",
            "TSCharactersExt.txt",
            "TSPhrasesBase.txt",
            "TSPhrasesExt.txt",
        )

        reports = []
        for file in glob.iglob(os.path.join(glob.escape(dict_dir), "*.txt")):
            basename = os.path.basename(file)
            if basename in excluded_dicts:
                continue

            table = Table.from_file(file)
            if sorted(table) != list(table):
                reports.append(basename)

        if reports:
            self.fail(
                "Dictionary files not sorted:\n" +
                "\n".join(reports)
            )

    def test_compatibility_ideographs(self):
        """
        Validate that regular dictionaries do not contain Unicode CJK
        Compatibility Ideographs.
        """
        excluded_dicts = (
            "TSCharactersBase",
            "TSCharactersExt",
            "TSPhrasesBase",
            "TSPhrasesExt",
        )

        reports = []
        for file in glob.iglob(os.path.join(glob.escape(dict_dir), "*.txt")):
            basename, _ = os.path.splitext(os.path.basename(file))
            if basename in excluded_dicts:
                continue

            for entry in Table().iter(file):
                reported_chars = set()
                for value in entry:
                    for char in value:
                        if char in reported_chars:
                            continue

                        cp = ord(char)
                        comp_entry = self.cjk_comp_table.get(cp)
                        if not comp_entry:
                            continue

                        reported_chars.add(char)
                        line = entry.line
                        rep_cp = comp_entry["std"]
                        if rep_cp:
                            reports.append(f"{basename}:{line}: U+{cp:04X} {char!r} (replace with U+{rep_cp:04X} {chr(rep_cp)!r})")
                        else:
                            reports.append(f"{basename}:{line}: U+{cp:04X} {char!r}")

        if reports:
            self.fail(
                "CJK compatibility ideographs validation failed.\n\n"
                "Regular dictionaries should not output Unicode CJK Compatibility Ideographs. "
                "Every compatibility ideograph should be replaced with its corresponding standard "
                "CJK unified ideograph form.\n\n" + "\n".join(reports)
            )

    def test_non_bmp(self):
        """
        Validate non-BMP characters in phrase and variant dictionaries.
        """
        excluded_dicts = (
            "TSCharacters",
            "TSCharactersBase",
            "TSCharactersExt",
            "TSPhrasesBase",
            "TSPhrasesExt",
            "STCharacters",
        )

        reports = []
        for file in glob.iglob(os.path.join(glob.escape(dict_dir), "*.txt")):
            basename, _ = os.path.splitext(os.path.basename(file))
            if basename in excluded_dicts:
                continue

            for entry in Table().iter(file):
                reported_chars = set()
                for value in entry:
                    for char in value:
                        if char in reported_chars:
                            continue

                        cp = ord(char)
                        if cp > 0xFFFF and char not in self.smp_table:
                            reported_chars.add(char)
                            reports.append(f"{basename}:{entry.line}: {char!r} (U+{cp:04X})")

        if reports:
            self.fail(
                "Unallowed SMP characters detected in dictionary values.\n\n"
                "If these characters are intended, please register them in "
                "'scheme/AllowedSmpChars.txt'.\n\n" + "\n".join(reports)
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

        reports = []
        for phrase_name, char_name in dict_pairs:
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

            for entry in Table().iter(phrase_file):
                key = entry.key
                for value in entry:
                    # @TODO: calculate length based on unicode composite units to
                    # allow a substitution like '<U+82A6>' => '<U+82A6><U+E0134>'
                    # or '𧍯' => '⿰虫风'.
                    if len(key) != len(value):
                        reports.append(
                            f"{phrase_name}:{entry.line}: "
                            f"{key!r} -> {value!r} has unmatching length."
                        )
                        continue

                    for k_char, v_char in zip(key, value):
                        if k_char == v_char:
                            continue

                        if v_char in char_cands.get(k_char, set()):
                            continue

                        reports.append(
                            f"{phrase_name}:{entry.line}: "
                            f"{k_char!r} -> {v_char!r} (in {key!r} -> {value!r}) "
                            f"not declared in {char_name}."
                        )

        if reports:
            self.fail(
                "Phrase-character dependency validation failed.\n\n"
                "Phrase-level character substitutions should also be declared in the "
                "corresponding character dictionary. If a direct character conversion "
                "is not generally desirable (e.g., 周 -> 週), make it a non-default "
                "candidate (e.g., 周 -> 周 週).\n" + "\n".join(reports)
            )

    def test_variant_rev_phrases(self):
        """Validate reverse variant phrase dictionary coverage."""
        dict_names = (
            "HKVariants",
            "TWVariants",
        )
        st_characters_file = os.path.join(dict_dir, "STCharacters.txt")
        st_phrases_file = os.path.join(dict_dir, "STPhrases.txt")

        reports = []
        for dict_name in dict_names:
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
                reports.append(
                    f"{phrases_file_name}:\n" + "\n".join(missing_phrases)
                )

        if reports:
            self.fail(
                "Reverse phrase identity exceptions validation failed.\n\n"
                "When a variant file declares an asymmetric mapping (e.g., 纔 -> 才) "
                "for characters that belong to the same ST variant set (e.g., 才 -> 才 纔), "
                "every corresponding output phrase in STPhrases (e.g., 专才 -> 專才) "
                "should be explicitly declared as an identity mapping (i.e., 專才 -> 專才) "
                "in the RevPhrases file to prevent an incorrect reverse conversion "
                "(i.e., 專才 -> 專纔).\n\n" + "\n\n".join(reports)
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

        reports = []
        for dict_name in dict_names:
            dict_name_rev = f"{dict_name}Rev"

            table = Table.from_file(os.path.join(dict_dir, f"{dict_name}.txt"))
            table_rev = Table.from_file(os.path.join(dict_dir, f"{dict_name_rev}.txt"))

            missing = []
            for key, values in table.items():
                for value in values:
                    if key not in table_rev.get(value, []):
                        missing.append(f"{value} -> {key}")
            if missing:
                reports.append(
                    f"{dict_name_rev}: expect following mappings:\n" + "\n".join(missing)
                )

            missing = []
            for key, values in table_rev.items():
                for value in values:
                    if key not in table.get(value, []):
                        missing.append(f"{value} -> {key}")
            if missing:
                reports.append(
                    f"{dict_name}: expect following mappings:\n" + "\n".join(missing)
                )

        if reports:
            self.fail(
                "Phrase reverse mapping validation failed.\n\n"
                "To ensure consistency, forward dictionaries and their reverse counterparts "
                "should maintain bijective mappings. For example, if TWPhrases contains "
                "'宏 -> 巨集', then TWPhrasesRev should contain '巨集 -> 宏'. "
                "If the reverse conversion is not desired, make an identity mapping "
                "(e.g., TWPhrases: '宏 -> 宏 巨集' and TWPhrasesRev: '巨集 -> 宏', '宏 -> 宏').\n\n" +
                "\n\n".join(reports)
            )

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

        reports = []
        for dict_name in dict_names:
            phrases_file = os.path.join(dict_dir, f"{dict_name}.txt")
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

                    reports.append(
                        f"{dict_name}:{entry.line}: {key!r} "
                        f"(may conflict with STPhrases.txt:{st_entry['line']}: "
                        f"{st_entry['key']!r} -> {value!r})"
                    )
                    break

        if reports:
            self.fail(
                "Phrase segmentation coverage validation failed.\n\n"
                "Every phrase key declared in a regional dictionary should be "
                "covered by the conversion values in STPhrases. "
                "If a regional key (e.g., 表達式 -> 運算式) contains an existing "
                "STPhrases value as a partial fragment (e.g., 表达 -> 表達), "
                "the full regional key (i.e., 表達式) should typically be added "
                "to STPhrases as well.\n\n"
                "Potentially missing STPhrases entries for segmentation:\n" + "\n".join(reports)
            )


if __name__ == "__main__":
    unittest.main()
