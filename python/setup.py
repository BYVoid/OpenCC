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

_this_dir = os.path.dirname(os.path.abspath(__file__))
_clib_dir = os.path.abspath(os.path.join(_this_dir, 'opencc', 'clib'))
_opencc_rootdir = os.path.abspath(os.path.join(_this_dir, '..'))

_cmake_file = os.path.join(_opencc_rootdir, 'CMakeLists.txt')
assert os.path.isfile(_cmake_file)

_build_dir = build_dir = os.path.join(_opencc_rootdir, 'build', 'python')
_libopenccfile = os.path.join(_clib_dir, 'lib', 'libopencc.so')


def _build_libopencc():
    if os.path.isfile(_libopenccfile):
        return  # Skip building binary file

    print('building libopencc')
    assert subprocess.call('command -v make', shell=True) == 0, \
        'Requires make'
    assert subprocess.call('command -v cmake', shell=True) == 0, \
        'Requires cmake'
    # Probably also needs to check for c-compilier

    errno = subprocess.call((
        'mkdir -p {build_dir};'
        'cd {build_dir};'
        'cmake '
        '-DBUILD_DOCUMENTATION:BOOL=OFF '
        '-DENABLE_GTEST:BOOL=OFF '
        '-DCMAKE_BUILD_TYPE=Release '
        '-DCMAKE_INSTALL_PREFIX={clib_dir} '
        '../..;'
        'make -j;'
        'make install;'
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


setuptools.setup(
    name='opencc',
    version='0.0.1',
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
