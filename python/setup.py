from __future__ import unicode_literals

import re
import os
import sys
import subprocess

import setuptools
import setuptools.command.build_py
import setuptools.command.install
import setuptools.command.develop
import setuptools.command.test

_clib_dir = os.path.join('opencc', 'clib')
_opencc_rootdir = '..'

_cmake_file = os.path.join(_opencc_rootdir, 'CMakeLists.txt')
assert os.path.isfile(_cmake_file)

_build_dir = build_dir = os.path.join(_opencc_rootdir, 'build', 'python')
_libopenccfile = os.path.join(_clib_dir, 'lib', 'libopencc.so')

_version_pattern = re.compile(r'OPENCC_VERSION_(MAJOR|MINOR|REVISION) (\d+)')
_version_file = os.path.join('opencc', 'version.py')


def _get_version_info():
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


def _write_version_file(version_info):
    with open(_version_file, 'w') as f:
        f.write('__version__ = "{}"'.format(version_info))


def _build_libopencc():
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


class BuildPyCommand(setuptools.command.build_py.build_py):
    def run(self):
        _build_libopencc()
        super().run()


class InstallCommand(setuptools.command.install.install):
    def run(self):
        _build_libopencc()
        super().run()


class DevelopCommand(setuptools.command.develop.develop):
    def run(self):
        _build_libopencc()
        super().run()


class PyTestCommand(setuptools.command.test.test):
    def run_tests(self):
        import pytest
        errno = pytest.main([])
        sys.exit(errno)


version_info = _get_version_info()
_write_version_file(version_info)

setuptools.setup(
    name='opencc',
    version=version_info,
    packages=['opencc'],
    package_data={'opencc': [
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
)
