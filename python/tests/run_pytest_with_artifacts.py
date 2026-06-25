import argparse
import os
import subprocess
import sys


def main() -> int:
    parser = argparse.ArgumentParser(
        description='Run pytest and capture both JUnit XML and console output.',
    )
    parser.add_argument(
        'paths',
        nargs='*',
        default=['python/tests'],
        help='Test paths passed to pytest.',
    )
    parser.add_argument(
        '--artifact-dir',
        default='python-test-artifacts',
        help='Directory for generated pytest artifacts.',
    )
    parser.add_argument(
        '--artifact-prefix',
        default='pytest',
        help='Filename prefix for generated pytest artifacts.',
    )
    args, passthrough = parser.parse_known_args()

    artifact_dir = os.path.abspath(args.artifact_dir)
    os.makedirs(artifact_dir, exist_ok=True)

    junit_path = os.path.join(artifact_dir, f'{args.artifact_prefix}.xml')
    log_path = os.path.join(artifact_dir, f'{args.artifact_prefix}.log')

    command = [
        sys.executable,
        '-m',
        'pytest',
        *args.paths,
        f'--junitxml={junit_path}',
        *passthrough,
    ]

    print(f'Running: {" ".join(command)}')
    print(f'Writing JUnit XML to: {junit_path}')
    print(f'Writing console log to: {log_path}')

    with open(log_path, 'w', encoding='utf-8', newline='') as log_file:
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        assert process.stdout is not None
        for line in process.stdout:
            sys.stdout.write(line)
            log_file.write(line)
        return process.wait()


if __name__ == '__main__':
    sys.exit(main())
