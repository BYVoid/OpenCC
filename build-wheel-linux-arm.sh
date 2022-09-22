apt update && apt install -y build-essential cmake

# Build and package
pip install -y --no-cache-dir setuptools wheel cmake
python setup.py build_ext bdist_wheel \
    --plat-name manylinux_2_17_aarch64

# Cleanup
rm -rf build python/opencc/clib OpenCC.egg-info