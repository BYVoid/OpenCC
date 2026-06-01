#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import io
import os
import tempfile
import unittest
from contextlib import redirect_stdout

from common import BaseTable, Table, RichTable, find_target_items, reverse_items, sort_items


class BaseTest(unittest.TestCase):
    def table_from(self, input_text_or_bytes):
        with tempfile.TemporaryDirectory() as tmpdir:
            src = os.path.join(tmpdir, "in.txt")

            with open(src, **(
                {"mode": "wb"} if isinstance(input_text_or_bytes, bytes) else
                {"mode": "w", "encoding": "utf-8"}
            )) as f:
                f.write(input_text_or_bytes)

            table = self.table_cls.from_file(src)

        return table

    def run_file_op(self, func, input_text_or_bytes):
        with tempfile.TemporaryDirectory() as tmpdir:
            src = os.path.join(tmpdir, "in.txt")
            dst = os.path.join(tmpdir, "out.txt")

            with open(src, **(
                {"mode": "wb"} if isinstance(input_text_or_bytes, bytes) else
                {"mode": "w", "encoding": "utf-8"}
            )) as f:
                f.write(input_text_or_bytes)

            func(src, dst)

            with open(dst, "rb") as f:
                return f.read()


class BaseTableTest(BaseTest):
    table_cls = BaseTable

    def test_init(self):
        table = self.table_cls()
        with tempfile.TemporaryDirectory() as tmpdir:
            dst = os.path.join(tmpdir, "out.txt")
            table.dump(dst)
            with open(dst, "rb") as f:
                result = f.read()

        self.assertEqual(result, b"")

    def test_raise_on_dup_key(self):
        with self.assertRaisesRegex(ValueError, "Duplicated key 'a' at line 2"):
            self.table_from("a\tA\na\tA\n")


class TableTest(BaseTableTest):
    table_cls = Table

    def test_add_to_exist_key_simple(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.add('a', 'A2')
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA A2\nb\tB\n")

    def test_add_to_exist_key_multi(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.add('a', ['A2', 'A3'])
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA A2 A3\nb\tB\n")

    def test_add_to_exist_key_skip_dups(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.add('a', ['A', 'A2'])
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA A2\nb\tB\n")

    def test_add_to_new_key_simple(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.add('c', 'C1')
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA\nb\tB\nc\tC1\n")

    def test_add_to_new_key_multi(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.add('c', ['C', 'C2'])
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA\nb\tB\nc\tC C2\n")

    def test_add_to_new_key_skip_dups(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.add('c', ['C', 'C2', 'C'])
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA\nb\tB\nc\tC C2\n")

    def test_delete_simple(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.delete('a', 'A1')
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA1 A2 A3\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA2 A3\nb\tB\n")

    def test_delete_multi(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.delete('a', ['A1', 'A3'])
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA1 A2 A3\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA2\nb\tB\n")

    def test_delete_remove_empty_entry(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.delete('a', ['A1', 'A2', 'A3'])
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA1 A2 A3\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "b\tB\n")

    def test_delete_skip_nonexist_values(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.delete('a', ['A3', 'A4'])
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA1 A2 A3\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA1 A2\nb\tB\n")

    def test_delete_skip_nonexist_key(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.delete('C', ['C'])
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA1 A2\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA1 A2\nb\tB\n")

    def test_remove_basic(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.remove('a')
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA1 A2 A3\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "b\tB\n")

    def test_remove_skip_nonexist_key(self):
        def func(input_filename, output_filename):
            table = self.table_cls.from_file(input_filename)
            table.remove('c')
            table.dump(output_filename)

        out = self.run_file_op(func, "a\tA1 A2 A3\nb\tB\n")
        self.assertEqual(out.decode("utf-8"), "a\tA1 A2 A3\nb\tB\n")


class RichTableTest(BaseTableTest):
    table_cls = RichTable


class CommonScriptTest(BaseTest):
    def test_sort_entries(self):
        out = self.run_file_op(sort_items, "b\tB\na\tA\n")
        self.assertEqual(out.decode("utf-8"), "a\tA\nb\tB\n")

    def test_sort_moves_comment_with_following_entry(self):
        out = self.run_file_op(sort_items, "b\tB\n# comment for a\na\tA\n")
        self.assertEqual(
            out.decode("utf-8"),
            "# comment for a\na\tA\nb\tB\n",
        )

    def test_sort_preserves_blank_lines_in_anchored_block(self):
        out = self.run_file_op(sort_items, "b\tB\n\n# comment for a\na\tA\n")
        self.assertEqual(
            out.decode("utf-8"),
            "\n# comment for a\na\tA\nb\tB\n",
        )

    def test_sort_splits_header_from_first_entry_comment(self):
        out = self.run_file_op(
            sort_items,
            "# Header\n\n# comment for b\nb\tB\na\tA\n",
        )
        self.assertEqual(
            out.decode("utf-8"),
            "# Header\n\na\tA\n# comment for b\nb\tB\n",
        )

    def test_sort_keeps_footer_at_end(self):
        out = self.run_file_op(sort_items, "b\tB\na\tA\n\n# footer\n")
        self.assertEqual(
            out.decode("utf-8"),
            "a\tA\nb\tB\n\n# footer\n",
        )

    def test_sort_preserves_missing_final_newline(self):
        out = self.run_file_op(sort_items, b"b\tB\na\tA")
        self.assertEqual(out, b"a\tA\nb\tB")

    def test_sort_normalizes_crlf_input(self):
        out = self.run_file_op(sort_items, b"b\tB\r\na\tA\r\n")
        self.assertEqual(out, b"a\tA\nb\tB\n")

    def test_sort_normalizes_cr_input(self):
        out = self.run_file_op(sort_items, b"b\tB\ra\tA\r")
        self.assertEqual(out, b"a\tA\nb\tB\n")

    def test_reverse_items(self):
        out = self.run_file_op(reverse_items, "A\tx y\nB\tx\n")
        self.assertEqual(
            out.decode("utf-8"),
            "x\tA B\ny\tA\n",
        )

    def test_find_target_items_matches_values_only(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            src = os.path.join(tmpdir, "in.txt")
            with open(src, "w", encoding="utf-8") as f:
                f.write("foo\tbar baz\n")
                f.write("target\txxx\n")
                f.write("abc\ttarget-value\n")

            buf = io.StringIO()
            with redirect_stdout(buf):
                find_target_items(src, "target")

            self.assertEqual(buf.getvalue(), "abc\ttarget-value\n")


if __name__ == "__main__":
    unittest.main()
