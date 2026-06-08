# -*- coding: utf-8 -*-

from collections import UserDict, UserList


class BaseTable(UserDict):
    def _iter(self, file):
        with open(file, 'r', encoding='utf-8') as fh:
            yield from fh

    def iter(self, file):
        for i, line in enumerate(self._iter(file)):
            if line.startswith('#') or not line.strip():
                continue
            key, values = line.rstrip('\n').split('\t', 1)
            entry = UserList(values.split(' '))
            entry.key = key
            entry.line = i + 1
            yield entry

    def load(self, file):
        data = {}
        for entry in self.iter(file):
            key = self.get_entry_key(entry)
            if key in data:
                line = getattr(entry, 'line', None)
                if line is not None:
                    raise ValueError(f'Duplicated key {key!r} at line {line}')
                else:
                    raise ValueError(f'Duplicated key {key!r}')
            data[key] = entry

        self.data = data

    def dump(self, file, sort=False):
        with open(file, 'wb') as fh:
            keys = sorted(self.data) if sort else self.data
            for key in keys:
                entry = self.data[key]
                fh.write(self.get_entry_line(entry).encode('utf-8'))

    @classmethod
    def from_file(cls, file):
        table = cls()
        table.load(file)
        return table

    @staticmethod
    def get_entry_key(entry):
        return entry.key

    @staticmethod
    def get_entry_line(entry):
        return entry.key + '\t' + ' '.join(entry) + '\n'


class Table(BaseTable):
    def swap(self):
        dic = {}
        for key, entry in self.items():
            for value in entry:
                if value in dic:
                    dic[value].append(key)
                else:
                    dic[value] = [key]

        data = {}
        for key, values in dic.items():
            entry = UserList(values)
            entry.key = key
            data[self.get_entry_key(entry)] = entry

        self.data = data

    def add(self, key, values):
        if isinstance(values, str):
            values = (values,)
        else:
            values = dict.fromkeys(values)

        if key in self.data:
            vv = self.data[key]
            vv.extend(v for v in values if v not in vv)
        else:
            entry = UserList(values)
            entry.key = key
            self.data[key] = entry

    def delete(self, key, values):
        if isinstance(values, str):
            values = (values,)

        if key in self.data:
            self.data[key][:] = (v for v in self.data[key] if v not in values)
            if not self.data[key]:
                del self.data[key]

    def remove(self, key):
        self.data.pop(key, None)


class RichTable(Table):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.header = None
        self.footer = None
        self.final_newline = True

    def load(self, file):
        header = None
        footer = None
        data = {}
        block = []
        _line = ''
        for i, _line in enumerate(self._iter(file)):
            line = _line.rstrip('\n')
            if self.get_line_type(line) == 'entry':
                key, values = line.split('\t', 1)
                if key in data:
                    raise ValueError(f'Duplicated key {repr(key)} at line {i + 1}')

                entry = UserList(values.split(' '))
                entry.key = key
                entry.line = i + 1
                if block:
                    if not data:
                        last_empty_idx = next(
                            (i for i in range(len(block) - 1, -1, -1) if not block[i].strip()), 
                            -1
                        )
                        block_anchored = block[last_empty_idx + 1:]
                        del block[last_empty_idx + 1:]
                        if block:
                            header = block
                        if block_anchored:
                            entry.block = block_anchored
                    else:
                        entry.block = block
                data[self.get_entry_key(entry)] = entry

                block = []

            else:
                block.append(line)
        else:
            if block:
                footer = block
            final_newline = _line.endswith('\n')

        self.header = header
        self.footer = footer
        self.data = data
        self.final_newline = final_newline

    def dump(self, file, sort=False):
        def write_block(fh, block):
            if block is None:
                return
            for line in block:
                fh.write((line + '\n').encode('utf-8'))

        def write_entry(fh, entry):
            fh.write(self.get_entry_line(entry).encode('utf-8'))

        items = self.items()
        if sort:
            items = sorted(items)

        with open(file, 'w+b') as fh:
            write_block(fh, self.header)
            for _, entry in items:
                write_block(fh, getattr(entry, 'block', None))
                write_entry(fh, entry)
            write_block(fh, self.footer)

            if not self.final_newline:
                pos = fh.tell()
                if pos > 0:
                    fh.seek(pos - 1)
                    if fh.read(1) == b'\n':
                        fh.truncate(pos - 1)

    @staticmethod
    def get_line_type(line):
        if not line.strip():
            return 'empty'
        if line.startswith('#'):
            return 'comment'
        if '\t' in line:
            return 'entry'
        raise ValueError('Invalid dictionary line: ' + line)


class CjkCompTable(BaseTable):
    def iter(self, file):
        for i, line in enumerate(self._iter(file)):
            if line.startswith('#') or not line.strip():
                continue
            comp_code, std_code = (line.rstrip('\n').split('\t') + [None] * 2)[:2]
            entry = UserDict({
                'comp': int(comp_code, 16),
                'std': int(std_code, 16) if std_code else None,
            })
            entry.line = i + 1
            yield entry

    @staticmethod
    def get_entry_key(entry):
        return entry['comp']

    @staticmethod
    def get_entry_line(entry):
        return f'U+{entry["comp"]:04X}' + '\t' + f'U+{entry["std"]:04X}' + '\n'


class SmpTable(BaseTable):
    def iter(self, file):
        for i, line in enumerate(self._iter(file)):
            if line.startswith('#') or not line.strip():
                continue
            char, rep = (line.rstrip('\n').split('\t') + [None] * 2)[:2]
            entry = UserDict({
                'char': char,
                'rep': rep or None,
            })
            entry.line = i + 1
            yield entry

    @staticmethod
    def get_entry_key(entry):
        return entry['char']

    @staticmethod
    def get_entry_line(entry):
        return entry['char'] + '\t' + entry.get('rep', '') + '\n'


def sort_items(input_filename, output_filename):
    table = RichTable.from_file(input_filename)
    table.dump(output_filename, sort=True)


def reverse_items(input_filename, output_filename):
    table = Table.from_file(input_filename)
    table.swap()
    table.dump(output_filename, sort=True)


def find_target_items(input_filename, keyword):
    for entry in Table().iter(input_filename):
        for value in entry:
            if keyword in value:
                print(Table.get_entry_line(entry), end='')
