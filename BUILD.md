

# Building Python distributables

This section contain guides to building distributable `.whl` files for the
purpose of distributing code on the
[Python Package Index (`PyPI`)](https://pypi.org/).

## Linux

It is highly recommended to generate distributable files using the provided
`Dockerfile.posix` file.
This also makes its easy to build from MacOS.

Requirements:
- [`docker`](https://www.docker.com/) (tested on `v19.03.8`)

```sh
# Building Docker image
# Run this from the root directory of this repository
docker build --rm -t opencc -f python/Dockerfile.posix .

# Mount ./dist and build `.whl` files
mkdir -p dist
docker run --rm -it -v $PWD/dist:/opt/OpenCC/dist opencc \
    bash bdist_array.bash /opt/conda/etc/profile.d/conda.sh

# It might be necessary to change the ownership of files
sudo chown -R $USER:$USER ./dist
```

## MacOS

As there are no MacOS docker images, distributable files have to be built
locally.

Requirements:
- [xcode](https://developer.apple.com/xcode/)
- An installation of [conda](https://docs.conda.io/en/latest/)
  (miniconda or anaconda)

```sh
# Ensure that miniconda is pre-installed
# Run this from the root directory of this repository
# replacing CONDA_INSTALL_DIR
bash ./bdist_array.bash $CONDA_INSTALL_DIR/etc/profile.d/conda.sh
```

## Windows

Requirements:
- Microsoft Visual C++
- `CMakeTools`
- An installation of [conda](https://docs.conda.io/en/latest/)
  (miniconda or anaconda)

> The requirements can be installed using Visual Studio Community Edition

```
# Ensure that `cmake.exe` and `conda` can all be found in your PATH
.\bdist_array.cmd
```
