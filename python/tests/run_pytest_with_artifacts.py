import argparse
import os
import subprocess
import sys


def _filter_pythonpath(repo_root: str) -> str:
    excluded = {
        os.path.normcase(os.path.abspath(repo_root)),
        os.path.normcase(os.path.abspath(os.path.join(repo_root, 'python'))),
    }
    entries = []
    for entry in os.environ.get('PYTHONPATH', '').split(os.pathsep):
        if not entry:
            continue
        normalized = os.path.normcase(os.path.abspath(entry))
        if normalized in excluded:
            continue
        entries.append(entry)
    return os.pathsep.join(entries)


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
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))

    junit_path = os.path.join(artifact_dir, f'{args.artifact_prefix}.xml')
    log_path = os.path.join(artifact_dir, f'{args.artifact_prefix}.log')
    test_paths = [os.path.abspath(path) for path in args.paths]

    env = os.environ.copy()
    filtered_pythonpath = _filter_pythonpath(repo_root)
    if filtered_pythonpath:
        env['PYTHONPATH'] = filtered_pythonpath
    else:
        env.pop('PYTHONPATH', None)

    command = [
        sys.executable,
        '-m',
        'pytest',
        *test_paths,
        f'--junitxml={junit_path}',
        *passthrough,
    ]

    print(f'Running: {" ".join(command)}')
    print(f'Writing JUnit XML to: {junit_path}')
    print(f'Writing console log to: {log_path}')
    print(f'Running from cwd: {artifact_dir}')

    with open(log_path, 'w', encoding='utf-8', newline='') as log_file:
        probe = subprocess.run(
            [
                sys.executable,
                '-c',
                'import opencc; print(opencc.__file__)',
            ],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            cwd=artifact_dir,
            env=env,
            check=False,
        )
        probe_output = probe.stdout.strip()
        import_line = f'opencc import probe: {probe_output or "<no output>"}\n'
        sys.stdout.write(import_line)
        log_file.write(import_line)
        if probe.returncode != 0:
            failure_line = f'opencc import probe failed with exit code {probe.returncode}\n'
            sys.stdout.write(failure_line)
            log_file.write(failure_line)

        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            cwd=artifact_dir,
            env=env,
        )
        assert process.stdout is not None
        for line in process.stdout:
            sys.stdout.write(line)
            log_file.write(line)
        return process.wait()


if __name__ == '__main__':
    sys.exit(main())
