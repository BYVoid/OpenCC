#!/usr/bin/env python3
"""Benchmark the pure Python OpenCC package with project data."""

import argparse
import json
import statistics
import sys
import time
from pathlib import Path

import opencc
from opencc import _opencc_pure

_this_dir = Path(__file__).resolve().parent
_opencc_rootdir = _this_dir.parents[1]
_testcases_path = _opencc_rootdir / 'test' / 'testcases' / 'testcases.json'
_golden_input_path = _opencc_rootdir / 'test' / 'golden' / 'input' / 'us_constitution_zhs.txt'
_long_input_path = _opencc_rootdir / 'test' / 'benchmark' / 'zuozhuan.txt'


def _config_name(config):
    return config[:-5] if config.endswith('.json') else config


def _default_configs():
    preferred = ['s2t', 't2s', 's2tw', 's2twp', 's2hk', 'tw2s']
    available = {
        _config_name(config)
        for config in opencc.CONFIGS
        if 'jieba' not in config
    }
    return [config for config in preferred if config in available]


def _load_texts(dataset):
    if dataset == 'testcases':
        with open(_testcases_path, encoding='utf-8') as f:
            parsed = json.load(f)
        return [
            case['input']
            for case in parsed.get('cases', [])
            if case.get('input')
        ]

    if dataset == 'golden':
        return [_golden_input_path.read_text(encoding='utf-8')]

    if dataset == 'long':
        return [_long_input_path.read_text(encoding='utf-8')]

    raise ValueError(f'Unknown dataset: {dataset}')


def _time_call(fn):
    start = time.perf_counter()
    result = fn()
    return time.perf_counter() - start, result


def _median(values):
    return statistics.median(values) if values else 0.0


def _benchmark_init(configs, iterations):
    rows = []
    for config in configs:
        samples = []
        for _ in range(iterations):
            _opencc_pure._trie_cache.clear()
            elapsed, _ = _time_call(lambda: opencc.OpenCC(config))
            samples.append(elapsed)
        rows.append({
            'name': f'init/{config}',
            'iterations': iterations,
            'seconds_median': _median(samples),
            'seconds_min': min(samples),
            'seconds_max': max(samples),
            'chars_per_second': None,
        })
    return rows


def _benchmark_convert(configs, texts, warmups, iterations):
    total_chars = sum(len(text) for text in texts)
    rows = []
    for config in configs:
        converter = opencc.OpenCC(config)
        for _ in range(warmups):
            for text in texts:
                converter.convert(text)

        samples = []
        for _ in range(iterations):
            elapsed, _ = _time_call(
                lambda: [converter.convert(text) for text in texts]
            )
            samples.append(elapsed)

        median = _median(samples)
        rows.append({
            'name': f'convert/{config}',
            'iterations': iterations,
            'seconds_median': median,
            'seconds_min': min(samples),
            'seconds_max': max(samples),
            'chars_per_second': total_chars / median if median > 0 else 0.0,
        })
    return rows


def _print_table(rows):
    print('| benchmark | iterations | median ms | min ms | max ms | chars/s |')
    print('|---|---:|---:|---:|---:|---:|')
    for row in rows:
        chars_per_second = row['chars_per_second']
        if chars_per_second is None:
            throughput = ''
        else:
            throughput = f'{chars_per_second:,.0f}'
        print(
            '| {name} | {iterations} | {median:.3f} | {min:.3f} | {max:.3f} | {throughput} |'.format(
                name=row['name'],
                iterations=row['iterations'],
                median=row['seconds_median'] * 1000,
                min=row['seconds_min'] * 1000,
                max=row['seconds_max'] * 1000,
                throughput=throughput,
            )
        )


def _print_json(rows):
    print(json.dumps(rows, ensure_ascii=False, indent=2))


def main(argv=None):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--dataset',
        choices=['testcases', 'golden', 'long'],
        default='golden',
        help='Project dataset to use for conversion benchmarks.',
    )
    parser.add_argument(
        '--configs',
        nargs='+',
        default=None,
        help='Config names to benchmark. Defaults to common non-jieba configs.',
    )
    parser.add_argument(
        '--warmups',
        type=int,
        default=1,
        help='Warmup conversion runs before timing.',
    )
    parser.add_argument(
        '--iterations',
        type=int,
        default=5,
        help='Timed iterations per benchmark.',
    )
    parser.add_argument(
        '--include-init',
        action='store_true',
        help='Also benchmark converter construction time.',
    )
    parser.add_argument(
        '--json',
        action='store_true',
        help='Emit machine-readable JSON instead of a Markdown table.',
    )
    args = parser.parse_args(argv)

    configs = [_config_name(config) for config in (args.configs or _default_configs())]
    texts = _load_texts(args.dataset)

    rows = []
    if args.include_init:
        rows.extend(_benchmark_init(configs, args.iterations))
    rows.extend(_benchmark_convert(configs, texts, args.warmups, args.iterations))

    if args.json:
        _print_json(rows)
    else:
        _print_table(rows)


if __name__ == '__main__':
    main(sys.argv[1:])
