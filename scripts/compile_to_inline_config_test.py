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


if __name__ == "__main__":
    unittest.main()
