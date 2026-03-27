import os
import re
import shutil
import subprocess
import sys
import warnings

import setuptools
import setuptools.command.build_ext
import wheel.bdist_wheel

_this_dir = os.path.dirname(os.path.abspath(__file__))
_build_dir = os.path.join(_this_dir, 'build', 'python')

_cmake_file = os.path.join(_this_dir, 'CMakeLists.txt')
_author_file = os.path.join(_this_dir, 'AUTHORS')
_readme_file = os.path.join(_this_dir, 'README.md')
_fallback_version = '1.2.1'


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


def build_libopencc(output_path):
    print('building libopencc into %s' % _build_dir)

    is_windows = sys.platform == 'win32'

    # Make build directories
    os.makedirs(_build_dir, exist_ok=True)

    # Configure
    cmake_args = [
        '-DBUILD_DOCUMENTATION:BOOL=OFF',
        '-DBUILD_SHARED_LIBS:BOOL=OFF',
        '-DENABLE_GTEST:BOOL=OFF',
        '-DENABLE_BENCHMARK:BOOL=OFF',
        '-DBUILD_PYTHON:BOOL=ON',
        '-DCMAKE_BUILD_TYPE=Release',
        '-DCMAKE_INSTALL_PREFIX={}'.format(output_path),
        '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={}'.format(output_path),
        '-DPYTHON_EXECUTABLE={}'.format(sys.executable),
    ]

    if is_windows:
        cmake_args += \
            ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE={}'.format(output_path)]
        if sys.maxsize > 2**32:
            cmake_args += ['-A', 'x64']

    cmd = ['cmake', '-B', _build_dir] + cmake_args
    errno = subprocess.call(cmd)
    assert errno == 0, 'Configure failed'

    # Build
    cmd = [
        'cmake', '--build', _build_dir,
        '--config', 'Release',
        '--target', 'install'
    ]
    errno = subprocess.call(cmd)
    assert errno == 0, 'Build failed'


class OpenCCExtension(setuptools.Extension, object):
    def __init__(self, name, sourcedir=''):
        setuptools.Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class BuildExtCommand(setuptools.command.build_ext.build_ext, object):
    def build_extension(self, ext):
        if self.inplace:
            output_path = os.path.join(_this_dir, 'python', 'opencc', 'clib')
        else:
            output_path = os.path.abspath(os.path.join(self.build_lib, 'opencc', 'clib'))
        if isinstance(ext, OpenCCExtension):
            build_libopencc(output_path)
        else:
            super(BuildExtCommand, self).build_extension(ext)


class BDistWheelCommand(wheel.bdist_wheel.bdist_wheel, object):
    """Custom bdsit_wheel command that will change
    default plat-name based on PEP 425 and PEP 513
    """

    @staticmethod
    def _determine_platform_tag():
        if sys.platform == 'win32':
            if 'amd64' in sys.version.lower():
                return 'win-amd64'
            return sys.platform

        if sys.platform == 'darwin':
            _, _, _, _, machine = os.uname()
            if machine == 'x86_64':
                return 'macosx-10.9-{}'.format(machine)
            if machine == 'arm64':
                return 'macosx-11.0-{}'.format(machine)
            else:
                raise NotImplementedError

        if os.name == 'posix':
            _, _, _, _, machine = os.uname()
            return 'manylinux2014-{}'.format(machine)

        warnings.warn(
            'Windows macos and linux are all not detected, '
            'Proper distribution name cannot be determined.')
        from distutils.util import get_platform
        return get_platform()

    def initialize_options(self):
        super(BDistWheelCommand, self).initialize_options()
        self.plat_name = self._determine_platform_tag()


packages = ['opencc', 'opencc.clib']

version_info = get_version_info()
author_info = get_author_info()

setup_requires = []
if not shutil.which('cmake'):
    setup_requires.append('cmake')

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
    ext_modules=[OpenCCExtension('opencc.clib.opencc_clib', 'python')],
    cmdclass={
        'build_ext': BuildExtCommand,
        'bdist_wheel': BDistWheelCommand,
    },
    setup_requires=setup_requires,

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
