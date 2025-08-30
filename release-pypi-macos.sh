#!/bin/sh
set -e

# Different to release-pypi-win.cmd and release-pypi-osx.sh,
# this script has to be ran from a clean dockerfile

# Download and init conda
# Detect architecture and set appropriate Miniconda filename
ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ]; then
    MINICONDA_FILENAME=Miniconda3-latest-MacOSX-arm64.sh
else
    MINICONDA_FILENAME=Miniconda3-latest-MacOSX-x86_64.sh
fi

curl -L -o $MINICONDA_FILENAME \
    "https://repo.continuum.io/miniconda/$MINICONDA_FILENAME"
bash ${MINICONDA_FILENAME} -b -f -p $HOME/miniconda3
export PATH=$HOME/miniconda3/bin:$PATH
eval "$(conda shell.bash hook)"

# Accept conda Terms of Service for required channels
conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main
conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/r

for VERSION in 3.8 3.9 3.10 3.11 3.12; do
    # Create and activate environment
    conda config --add channels conda-forge
    conda config --set channel_priority strict
    conda create -y -n py$VERSION python=$VERSION
    conda activate py$VERSION

    # Build and package
    pip install --no-cache-dir build
    python -m build --wheel

    # Cleanup
    conda deactivate
    rm -rf build OpenCC.egg-info
done

if [ "$1" != "testonly" ]; then
    # Upload to PyPI
    conda activate py3.8
    python -m pip install twine
    python -m twine upload dist/*
fi
