@echo off
setlocal EnableDelayedExpansion

SET VERSIONS=3.7 3.8 3.9 3.10
SET SOURCEDIR=%cd%

REM Build packages
for %%v in (%VERSIONS%) do (
    SET ENV_NAME=py%%v

    REM Create and activate environment
    cd %ROOT_DIR%
    CALL C:\Miniconda/condabin/conda.bat create -y -n py%%v python=%%v
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
    CALL C:\Miniconda/condabin/conda.bat activate py%%v
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
    pip install --no-cache-dir setuptools wheel pytest
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

    REM Build and package
    python setup.py build_ext bdist_wheel
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

    REM Cleanup
    CALL C:\Miniconda/condabin/conda.bat deactivate
    rmdir /S /Q build python\opencc\clib OpenCC.egg-info
)

REM Upload to PyPI
C:\Miniconda/condabin/conda.bat activate py3.8
python -m pip install twine
python -m twine upload dist/*
