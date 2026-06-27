import tempfile
import unittest
from pathlib import Path

from compile_to_inline_config import compile_config


class CompileToInlineConfigTest(unittest.TestCase):
    def test_group_dict_preserves_structure_and_match_policy(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            dict_dir = root / "dict"
            dict_dir.mkdir()
            (dict_dir / "First.txt").write_text("a\tX\n", encoding="utf-8")
            (dict_dir / "Second.txt").write_text("ab\tY\n", encoding="utf-8")

            config_path = root / "short_first.json"
            config_path.write_text(
                """
{
  "name": "short-first group test",
  "segmentation": {
    "type": "mmseg",
    "dict": {
      "type": "group",
      "match_policy": "short_circuit",
      "dicts": [
        { "type": "text", "file": "First.txt" },
        { "type": "text", "file": "Second.txt" }
      ]
    }
  },
  "conversion_chain": [
    {
      "dict": {
        "type": "group",
        "match_policy": "short_circuit",
        "dicts": [
          { "type": "text", "file": "First.txt" },
          { "type": "text", "file": "Second.txt" }
        ]
      }
    }
  ]
}
""",
                encoding="utf-8",
            )

            compiled = compile_config(config_path, dict_dir)

        inlined_dicts = [
            {"type": "inline", "entries": {"a": "X"}},
            {"type": "inline", "entries": {"ab": "Y"}},
        ]

        segmentation_dict = compiled["segmentation"]["dict"]
        self.assertEqual("group", segmentation_dict["type"])
        self.assertEqual("short_circuit", segmentation_dict["match_policy"])
        self.assertEqual(inlined_dicts, segmentation_dict["dicts"])

        conversion_dict = compiled["conversion_chain"][0]["dict"]
        self.assertEqual("group", conversion_dict["type"])
        self.assertEqual("short_circuit", conversion_dict["match_policy"])
        self.assertEqual(inlined_dicts, conversion_dict["dicts"])

    def test_group_dict_without_match_policy_compiles_cleanly(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            dict_dir = root / "dict"
            dict_dir.mkdir()
            (dict_dir / "Only.txt").write_text("x\tY\n", encoding="utf-8")

            config_path = root / "no_policy.json"
            config_path.write_text(
                """
{
  "name": "no match_policy test",
  "conversion_chain": [
    {
      "dict": {
        "type": "group",
        "dicts": [
          { "type": "text", "file": "Only.txt" }
        ]
      }
    }
  ]
}
""",
                encoding="utf-8",
            )

            compiled = compile_config(config_path, dict_dir)

        conversion_dict = compiled["conversion_chain"][0]["dict"]
        self.assertEqual("group", conversion_dict["type"])
        self.assertNotIn("match_policy", conversion_dict)


class NestedGroupPolicyTest(unittest.TestCase):
    """Two-level group nesting: union×union, short_circuit×union, union×short_circuit."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        root = Path(self._tmp.name)
        self.dict_dir = root / "dict"
        self.dict_dir.mkdir()
        self.config_dir = root / "config"
        self.config_dir.mkdir()
        # "x" appears in both A and C to verify first-occurrence wins.
        (self.dict_dir / "A.txt").write_text("a\tA_val\nx\tX_from_A\n", encoding="utf-8")
        (self.dict_dir / "B.txt").write_text("b\tB_val\n", encoding="utf-8")
        (self.dict_dir / "C.txt").write_text("c\tC_val\nx\tX_from_C\n", encoding="utf-8")

    def tearDown(self):
        self._tmp.cleanup()

    def _config(self, name: str, body: str) -> Path:
        path = self.config_dir / name
        path.write_text(
            f'{{"name": "{name}", "conversion_chain": [{{"dict": {body}}}]}}',
            encoding="utf-8",
        )
        return path

    def _dict(self, compiled) -> dict:
        return compiled["conversion_chain"][0]["dict"]

    def test_union_of_union_flattens_to_single_inline(self):
        # union( union(A, B), C ) → one flat inline; "x" from A wins over C.
        config = self._config(
            "union_of_union.json",
            """{
              "type": "group", "match_policy": "union",
              "dicts": [
                { "type": "group", "match_policy": "union",
                  "dicts": [
                    {"type": "text", "file": "A.txt"},
                    {"type": "text", "file": "B.txt"}
                  ]
                },
                {"type": "text", "file": "C.txt"}
              ]
            }""",
        )
        d = self._dict(compile_config(config, self.dict_dir))
        self.assertEqual("inline", d["type"])
        self.assertEqual(
            {"a": "A_val", "b": "B_val", "c": "C_val", "x": "X_from_A"},
            d["entries"],
        )

    def test_short_circuit_of_union_keeps_outer_group(self):
        # short_circuit( union(A, B), C ) → outer group is preserved;
        # the inner union child is compiled down to a merged inline.
        config = self._config(
            "sc_of_union.json",
            """{
              "type": "group", "match_policy": "short_circuit",
              "dicts": [
                { "type": "group", "match_policy": "union",
                  "dicts": [
                    {"type": "text", "file": "A.txt"},
                    {"type": "text", "file": "B.txt"}
                  ]
                },
                {"type": "text", "file": "C.txt"}
              ]
            }""",
        )
        d = self._dict(compile_config(config, self.dict_dir))
        self.assertEqual("group", d["type"])
        self.assertEqual("short_circuit", d["match_policy"])
        self.assertEqual(2, len(d["dicts"]))
        # Inner union(A, B) flattens; "x" from A wins over B (B has no "x").
        inner = d["dicts"][0]
        self.assertEqual("inline", inner["type"])
        self.assertEqual({"a": "A_val", "b": "B_val", "x": "X_from_A"}, inner["entries"])
        # C stays as its own inline.
        outer_c = d["dicts"][1]
        self.assertEqual("inline", outer_c["type"])
        self.assertEqual({"c": "C_val", "x": "X_from_C"}, outer_c["entries"])

    def test_union_of_short_circuit_falls_back_to_group(self):
        # union( short_circuit(A, B), C ) → union cannot absorb short_circuit;
        # falls back to a union group; the sc child is compiled but kept as group.
        config = self._config(
            "union_of_sc.json",
            """{
              "type": "group", "match_policy": "union",
              "dicts": [
                { "type": "group", "match_policy": "short_circuit",
                  "dicts": [
                    {"type": "text", "file": "A.txt"},
                    {"type": "text", "file": "B.txt"}
                  ]
                },
                {"type": "text", "file": "C.txt"}
              ]
            }""",
        )
        d = self._dict(compile_config(config, self.dict_dir))
        self.assertEqual("group", d["type"])
        self.assertEqual("union", d["match_policy"])
        self.assertEqual(2, len(d["dicts"]))
        # The short_circuit child is compiled (file→inline) but its group is kept.
        sc_child = d["dicts"][0]
        self.assertEqual("group", sc_child["type"])
        self.assertEqual("short_circuit", sc_child["match_policy"])
        self.assertEqual(
            [
                {"type": "inline", "entries": {"a": "A_val", "x": "X_from_A"}},
                {"type": "inline", "entries": {"b": "B_val"}},
            ],
            sc_child["dicts"],
        )
        # C is compiled to inline.
        c_child = d["dicts"][1]
        self.assertEqual("inline", c_child["type"])
        self.assertEqual({"c": "C_val", "x": "X_from_C"}, c_child["entries"])


if __name__ == "__main__":
    unittest.main()
