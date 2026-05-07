#!/bin/sh
set -e

python -m pip install --upgrade pip build pytest twine

python -m build --sdist --wheel --outdir wheelhouse
python -m pip install wheelhouse/*.whl
pytest python/tests

if [ "$1" != "testonly" ]; then
    # Upload to PyPI
    python -m twine upload wheelhouse/*
fi
