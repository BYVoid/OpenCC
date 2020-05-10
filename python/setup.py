from __future__ import unicode_literals

import os
import re
import subprocess
import sys

import setuptools
import setuptools.command.build_py
import setuptools.command.develop
import setuptools.command.install
import setuptools.command.test

from opencc import _libopenccfile

_clib_dir = os.path.join('opencc', 'clib')
_opencc_rootdir = '..'

_cmake_file = os.path.join(_opencc_rootdir, 'CMakeLists.txt')
assert os.path.isfile(_cmake_file)

_build_dir = build_dir = os.path.join(_opencc_rootdir, 'build', 'python')

_version_pattern = re.compile(r'OPENCC_VERSION_(MAJOR|MINOR|REVISION) (\d+)')
_version_file = os.path.join('opencc', 'version.py')


def get_version_info():
    version_info = ['1', '0', '0']
    with open(_cmake_file, 'rb') as f:
        for l in f:
            match = _version_pattern.search(l.decode('utf-8'))
            if not match:
                continue
            if match.group(1) == 'MAJOR':
                version_info[0] = match.group(2)
            elif match.group(1) == 'MINOR':
                version_info[1] = match.group(2)
            elif match.group(1) == 'REVISION':
                version_info[2] = match.group(2)
    return '.'.join(version_info)


def write_version_file(version_info):
    with open(_version_file, 'w') as f:
        f.write('__version__ = "{}"'.format(version_info))


def get_author_info():
    author_file = os.path.join(_opencc_rootdir, 'AUTHORS')
    if not os.path.isfile(author_file):
        return 'BYVoid', 'byvoid@byvoid.com'

    authors = []
    emails = []
    author_pattern = re.compile(r'(.+) <(.+)>')
    with open(author_file, 'rb') as f:
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
    with open(os.path.join(_opencc_rootdir, 'README.md'), 'rb') as f:
        return f.read().decode('utf-8')


def build_libopencc():
    if os.path.isfile(_libopenccfile):
        return  # Skip building binary file

    print('building libopencc')
    assert subprocess.call('command -v make', shell=True) == 0, \
        'Build requires `make`'
    assert subprocess.call('command -v cmake', shell=True) == 0, \
        'Build requires `cmake`'
    # Probably also needs to check for cpp-compilier

    errno = subprocess.call((
        'mkdir -p {build_dir};'
        'cmake '
        '-B {build_dir} '
        '-DBUILD_DOCUMENTATION:BOOL=OFF '
        '-DENABLE_GTEST:BOOL=OFF '
        '-DCMAKE_BUILD_TYPE=Release '
        '-DCMAKE_INSTALL_PREFIX={clib_dir} '
        '..;'
        'make -C {build_dir} -j;'
        'make -C {build_dir} install;'
    ).format(
        build_dir=_build_dir,
        clib_dir=_clib_dir
    ), shell=True)

    assert errno == 0, 'Build failed'
    assert os.path.isfile(_libopenccfile)


class BuildPyCommand(setuptools.command.build_py.build_py, object):
    def run(self):
        build_libopencc()
        super(BuildPyCommand, self).run()


class InstallCommand(setuptools.command.install.install, object):
    def run(self):
        build_libopencc()
        super(InstallCommand, self).run()


class DevelopCommand(setuptools.command.develop.develop, object):
    def run(self):
        build_libopencc()
        super(DevelopCommand, self).run()


class PyTestCommand(setuptools.command.test.test):
    def run_tests(self):
        import pytest
        errno = pytest.main([])
        sys.exit(errno)


version_info = get_version_info()
write_version_file(version_info)

author_info = get_author_info()

setuptools.setup(
    name='opencc-py',
    version=version_info,
    author=author_info[0],
    author_email=author_info[1],
    description=" Conversion between Traditional and Simplified Chinese",
    long_description=get_long_description(),
    long_description_content_type="text/markdown",
    url="https://github.com/BYVoid/OpenCC",

    packages=['opencc'],
    package_data={str('opencc'): [
        'clib/include/opencc/*',
        'clib/lib/libopencc.*',
        'clib/share/opencc/*',
    ]},
    cmdclass={
        'build_py': BuildPyCommand,
        'install': InstallCommand,
        'develop': DevelopCommand,
        'test': PyTestCommand,
    },

    tests_require=['pytest'],
    test_suite='tests',

    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
        'License :: OSI Approved :: Apache Software License',
        'Topic :: Scientific/Engineering',
        'Topic :: Software Development',
        'Topic :: Software Development :: Libraries',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Software Development :: Localization',
    ],
    license='Apache License 2.0',
    keywords='opencc convert chinese'
)
