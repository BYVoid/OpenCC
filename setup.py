import json
import os
import re
import shutil
import subprocess

import setuptools
import setuptools.command.build_py

_this_dir = os.path.dirname(os.path.abspath(__file__))

_author_file = os.path.join(_this_dir, 'AUTHORS')
_readme_file = os.path.join(_this_dir, 'README.md')
_fallback_version = '1.3.1'


def _get_version_from_git():
    try:
        raw = subprocess.check_output(
            ['git', 'describe', '--tags', '--long', '--always'],
            cwd=_this_dir,
            stderr=subprocess.DEVNULL,
        ).decode('utf-8').strip()
    except (OSError, subprocess.CalledProcessError):
        return ''

    dirty = ''
    for diff_cmd in (['git', 'diff', '--quiet'], ['git', 'diff', '--cached', '--quiet']):
        try:
            subprocess.check_call(
                diff_cmd,
                cwd=_this_dir,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        except (OSError, subprocess.CalledProcessError):
            dirty = '.dirty'
            break

    release_match = re.match(r'^(?:v|ver\.)(\d+\.\d+\.\d+)-0-g[0-9a-f]+$', raw)
    if release_match:
        return '{}{}'.format(release_match.group(1), dirty)

    dev_match = re.match(r'^(?:v|ver\.)(\d+\.\d+\.\d+)-(\d+)-g([0-9a-f]+)$', raw)
    if dev_match:
        return '{}.dev{}+g{}{}'.format(
            dev_match.group(1),
            dev_match.group(2),
            dev_match.group(3),
            dirty,
        )

    return '{}+g{}{}'.format(_fallback_version, raw, dirty)


def get_version_info():
    # Prefer Git-derived version from environment (set by CI or compute-version.sh)
    env_version = os.environ.get('VERSION', '')
    if env_version:
        # Strip leading 'v' prefix for PEP 440 compatibility
        return env_version.lstrip('v')

    git_version = _get_version_from_git()
    if git_version:
        return git_version

    return _fallback_version


def get_author_info():
    if not os.path.isfile(_author_file):
        return 'BYVoid', 'byvoid@byvoid.com'

    authors = []
    emails = []
    author_pattern = re.compile(r'(.+) <(.+)>')
    with open(_author_file, 'rb') as f:
        for line in f:
            match = author_pattern.search(line.decode('utf-8'))
            if not match:
                continue
            authors.append(match.group(1))
            emails.append(match.group(2))

    if len(authors) == 0:
        return 'BYVoid', 'byvoid@byvoid.com'

    return ', '.join(authors), ', '.join(emails)


def get_long_description():
    with open(_readme_file, 'rb') as f:
        return f.read().decode('utf-8')


class BuildPyCommand(setuptools.command.build_py.build_py, object):
    """Bundle OpenCC's JSON configs and text dictionaries into the package."""

    def run(self):
        super(BuildPyCommand, self).run()
        self._copy_opencc_data()

    def _copy_opencc_data(self):
        src_config = os.path.join(_this_dir, 'data', 'config')
        src_dict = os.path.join(_this_dir, 'data', 'dictionary')
        src_jieba_config = os.path.join(_this_dir, 'plugins', 'jieba', 'data', 'config')
        src_jieba_dict = os.path.join(_this_dir, 'plugins', 'jieba', 'deps', 'cppjieba', 'dict')
        dst_base = os.path.join(self.build_lib, 'opencc')
        dst_config = os.path.join(dst_base, 'config')
        dst_dict = os.path.join(dst_base, 'dictionary')
        dst_jieba_dict = os.path.join(dst_base, 'jieba_dict')

        os.makedirs(dst_config, exist_ok=True)
        for fname in os.listdir(src_config):
            if not fname.endswith('.json'):
                continue
            src_config_path = os.path.join(src_config, fname)
            with open(src_config_path, encoding='utf-8') as f:
                config = json.load(f)
            if config.get('segmentation', {}).get('type') != 'mmseg':
                continue
            shutil.copy2(src_config_path, os.path.join(dst_config, fname))
        if os.path.isdir(src_jieba_config):
            for fname in os.listdir(src_jieba_config):
                if fname.endswith('.json'):
                    shutil.copy2(
                        os.path.join(src_jieba_config, fname),
                        os.path.join(dst_config, fname),
                    )

        os.makedirs(dst_dict, exist_ok=True)
        for fname in os.listdir(src_dict):
            if fname.endswith('.txt'):
                shutil.copy2(
                    os.path.join(src_dict, fname),
                    os.path.join(dst_dict, fname),
                )
        os.makedirs(dst_jieba_dict, exist_ok=True)
        for fname in ('jieba.dict.utf8', 'user.dict.utf8'):
            src_path = os.path.join(src_jieba_dict, fname)
            if os.path.isfile(src_path):
                shutil.copy2(src_path, os.path.join(dst_jieba_dict, fname))


packages = ['opencc']

version_info = get_version_info()
author_info = get_author_info()

setuptools.setup(
    name='OpenCC',
    version=version_info,
    author=author_info[0],
    author_email=author_info[1],
    description=" Conversion between Traditional and Simplified Chinese",
    long_description=get_long_description(),
    long_description_content_type="text/markdown",
    url="https://github.com/BYVoid/OpenCC",

    packages=packages,
    package_dir={'opencc': 'python/opencc'},
    package_data={
        'opencc': ['py.typed', 'config/*.json', 'dictionary/*.txt', 'jieba_dict/*.utf8'],
    },
    cmdclass={
        'build_py': BuildPyCommand,
    },

    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'Natural Language :: Chinese (Simplified)',
        'Natural Language :: Chinese (Traditional)',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'License :: OSI Approved :: Apache Software License',
        'Topic :: Scientific/Engineering',
        'Topic :: Software Development',
        'Topic :: Software Development :: Libraries',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Software Development :: Localization',
        'Topic :: Text Processing :: Linguistic',
        'Typing :: Typed',
    ],
    license='Apache License 2.0',
    keywords=['opencc', 'convert', 'chinese'],
)
