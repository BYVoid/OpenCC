# -*- coding: utf-8 -*-

class BaseDict:
    def __init__(self):
        self.entries = []

    def _iter(self, file):
        with open(file, 'r', encoding='utf-8') as fh:
            yield from fh

    def iter(self, file):
        for i, line in enumerate(self._iter(file)):
            if line.startswith('#') or not line.strip():
                continue
            key, value = line.rstrip('\n').split('\t', 1)
            yield {
                'key': key,
                'values': value.split(' '),
                'line': i + 1,
            }

    def load(self, file):
        self.entries = list(self.iter(file))

    def dump(self, file):
        with open(file, 'wb') as fh:
            for entry in self.entries:
                fh.write(self.get_entry_line(entry).encode('utf-8'))

    @classmethod
    def from_file(cls, file):
        table = cls()
        table.load(file)
        return table

    @staticmethod
    def get_entry_line(entry):
        return entry['key'] + '\t' + ' '.join(entry['values']) + '\n'


class Dict(BaseDict):
    def sort(self):
        self.entries.sort(key=lambda e: e['key'])

    def swap(self):
        dic = {}
        for entry in self.entries:
            key = entry['key']
            for value in entry['values']:
                if value in dic:
                    dic[value].append(key)
                else:
                    dic[value] = [key]
        self.entries = [{'key': key, 'values': values} for key, values in dic.items()]


class RichDict(Dict):
    def __init__(self):
        super().__init__()
        self.blocks = {}
        self.final_newline = True

    def load(self, file):
        entries = []
        block_map = {}
        block = []
        _line = ''
        for i, _line in enumerate(self._iter(file)):
            line = _line.rstrip('\n')
            if self.get_line_type(line) == 'entry':
                entry_index = len(entries)

                if block:
                    if entry_index == 0:
                        last_empty_idx = next(
                            (i for i in range(len(block) - 1, -1, -1) if not block[i].strip()), 
                            -1
                        )
                        block_anchored = block[last_empty_idx + 1:]
                        del block[last_empty_idx + 1:]
                        if block:
                            block_map[-1] = block
                        if block_anchored:
                            block_map[0] = block_anchored
                    else:
                        block_map[entry_index] = block

                key, value = line.split('\t', 1)
                entries.append({
                    'key': key,
                    'values': value.split(' '),
                    'line': i + 1,
                    'orig_index': entry_index,
                })

                block = []

            else:
                block.append(line)
        else:
            if block:
                block_map[None] = block
            final_newline = _line.endswith('\n')

        self.entries = entries
        self.blocks = block_map
        self.final_newline = final_newline

    def dump(self, file):
        def write_block(fh, block):
            if block is None:
                return
            for line in block:
                fh.write((line + '\n').encode('utf-8'))

        def write_entry(fh, entry):
            fh.write(self.get_entry_line(entry).encode('utf-8'))

        index_map = {-1: -1, None: None}
        for i, e in enumerate(self.entries):
            idx = e.get('orig_index')
            if idx is not None:
                index_map[idx] = i
        blocks = {index_map[i]: block for i, block in self.blocks.items() if i in index_map}

        with open(file, 'w+b') as fh:
            write_block(fh, blocks.get(-1))
            for i, entry in enumerate(self.entries):
                write_block(fh, blocks.get(i))
                write_entry(fh, entry)
            write_block(fh, blocks.get(None))

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


def sort_items(input_filename, output_filename):
    table = RichDict.from_file(input_filename)
    table.sort()
    table.dump(output_filename)


def reverse_items(input_filename, output_filename):
    table = Dict.from_file(input_filename)
    table.swap()
    table.sort()
    table.dump(output_filename)


def find_target_items(input_filename, keyword):
    for entry in Dict().iter(input_filename):
        for value in entry['values']:
            if keyword in value:
                print(Dict.get_entry_line(entry), end='')
