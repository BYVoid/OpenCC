#!/bin/sh
set -e

# Linux wheels are built inside manylinux2014 containers managed by cibuildwheel.
python -m pip install --upgrade pip cibuildwheel twine build

export CIBW_BUILD="cp310-* cp311-* cp312-* cp313-* cp314-*"
export CIBW_SKIP="*-musllinux_*"
export CIBW_MANYLINUX_X86_64_IMAGE="manylinux2014"
export CIBW_MANYLINUX_AARCH64_IMAGE="manylinux2014"
export CIBW_ENVIRONMENT="VERSION=\"${VERSION}\""
export CIBW_TEST_REQUIRES="pytest"
export CIBW_TEST_COMMAND="pytest {project}/python/tests"

python -m cibuildwheel --output-dir wheelhouse

if [ "$(uname -m)" = "x86_64" ]; then
    # Build sdist only once, to avoid duplicate uploads from matrix jobs.
    python -m build --sdist --outdir wheelhouse
fi

if [ "$1" != "testonly" ]; then
    # Upload to PyPI
    python -m twine upload wheelhouse/*
fi
