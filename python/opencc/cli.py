import argparse
import os
import sys

import opencc


def _build_parser():
    config_list = '\n'.join(f'  {c}' for c in sorted(opencc.CONFIGS)) or '  (none found)'
    parser = argparse.ArgumentParser(
        prog='opencc',
        description='Open Chinese Convert (OpenCC) Command Line Tool',
        epilog=f'Built-in configurations:\n{config_list}',
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        '-c', '--config',
        default='s2t.json',
        metavar='<file>',
        help='Configuration file (default: s2t.json)',
    )
    parser.add_argument(
        '-i', '--input',
        metavar='<file>',
        help='Input file (default: stdin)',
    )
    parser.add_argument(
        '-o', '--output',
        metavar='<file>',
        help='Output file (default: stdout)',
    )
    parser.add_argument(
        '--resource-zip',
        metavar='<file>',
        help='Load config and dictionaries from a resource zip file',
    )
    parser.add_argument(
        '--include-tofu-risk-dictionaries',
        action='store_true',
        default=False,
        help='Include dictionaries that may output tofu characters',
    )
    parser.add_argument(
        '-v', '--version',
        action='version',
        version=f'OpenCC {opencc.__version__}',
    )
    return parser


_NATIVE_CLI_ONLY_FLAGS = ('--inspect', '--segmentation', '--ambiguities')


def main():
    for arg in sys.argv[1:]:
        flag = arg.split('=', 1)[0]
        if flag in _NATIVE_CLI_ONLY_FLAGS:
            print(
                f'opencc: error: {flag} is not supported by the Python CLI. '
                'Use the native OpenCC CLI instead.',
                file=sys.stderr,
            )
            sys.exit(1)

    parser = _build_parser()
    args = parser.parse_args()

    if args.input and args.output:
        try:
            if os.path.samefile(args.input, args.output):
                print('opencc: error: input and output refer to the same file', file=sys.stderr)
                sys.exit(1)
        except FileNotFoundError:
            pass

    converter = opencc.OpenCC(
        args.config,
        include_tofu_risk_dictionaries=args.include_tofu_risk_dictionaries,
        resource_zip=args.resource_zip,
    )

    if args.input:
        with open(args.input, 'rb') as f:
            text = f.read().decode('utf-8')
    else:
        text = sys.stdin.buffer.read().decode('utf-8')

    result = converter.convert(text)

    if args.output:
        with open(args.output, 'wb') as f:
            f.write(result.encode('utf-8'))
    else:
        sys.stdout.buffer.write(result.encode('utf-8'))


if __name__ == '__main__':
    main()
