#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from common import Table, SmpTable


def separate_tofu_risk(input_path, scheme_path, output_base_path, output_ext_path):
    scheme_table = SmpTable.from_file(scheme_path)

    table = Table.from_file(input_path)
    table_base = Table()
    table_ext = Table()

    for key, values in table.items():
        values_base = []
        values_ext = []
        for value in values:
            rep = ''.join(
                (scheme_table.get(c, {}).get('rep') or c) if ord(c) > 0xFFFF else c
                for c in value
            )
            values_base.append(rep)
            values_ext.append(value)

        if values_base:
            table_base.add(key, values_base)
        if values_ext and values_ext != values_base:
            table_ext.add(key, values_ext)

    table_base.dump(output_base_path)
    table_ext.dump(output_ext_path)


if len(sys.argv) != 5:
    print("Separate files based on tofu-risk")
    print(("Usage: ", sys.argv[0], "[input] [scheme] [output1] [output2]"))
    exit(1)

separate_tofu_risk(*sys.argv[1:])
