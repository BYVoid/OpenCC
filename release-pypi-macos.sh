#!/bin/sh
set -e

python -m pip install --upgrade pip cibuildwheel twine

export CIBW_BUILD="cp310-* cp311-* cp312-* cp313-* cp314-*"
export CIBW_ENVIRONMENT="VERSION=\"${VERSION}\""
export CIBW_TEST_REQUIRES="pytest"
export CIBW_TEST_COMMAND="pytest {project}/python/tests"

python -m cibuildwheel --output-dir wheelhouse

if [ "$1" != "testonly" ]; then
    # Upload to PyPI
    python -m twine upload wheelhouse/*.whl
fi
