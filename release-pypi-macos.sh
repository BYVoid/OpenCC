#!/bin/sh
set -e

# Different to release-pypi-win.cmd and release-pypi-osx.sh,
# this script has to be ran from a clean dockerfile

# Download and init conda
MINICONDA_FILENAME=Miniconda3-latest-MacOSX-x86_64.sh
curl -L -o $MINICONDA_FILENAME \
    "https://repo.continuum.io/miniconda/$MINICONDA_FILENAME"
bash ${MINICONDA_FILENAME} -b -f -p $HOME/miniconda3
export PATH=$HOME/miniconda3/bin:$PATH
eval "$(conda shell.bash hook)"

for VERSION in 3.7 3.8 3.9 3.10; do
    # Create and activate environment
    conda create -y -n py$VERSION python=$VERSION
    conda activate py$VERSION

    # Build and package
    pip install --no-cache-dir setuptools wheel
    python setup.py build_ext bdist_wheel

    # Cleanup
    conda deactivate
    rm -rf build python/opencc/clib OpenCC.egg-info
done

# Upload to PyPI
conda activate py3.8
python -m pip install twine
python -m twine upload dist/*
