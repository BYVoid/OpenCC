@echo off
setlocal EnableDelayedExpansion

SET VERSIONS=2.7 3.5 3.6 3.7 3.8
SET SOURCEDIR=%cd%

REM Build packages
for %%v in (%VERSIONS%) do (
    SET ENV_NAME=py%%v

    REM Create and activate environment
    cd %ROOT_DIR%
    CALL conda create -y -n py%%v python=%%v
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
    CALL conda activate py%%v
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
    pip install --no-cache-dir setuptools wheel pytest
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

    REM Build and package
    python setup.py build_ext bdist_wheel
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

    REM Cleanup
    CALL conda deactivate
    rmdir /S /Q build python\opencc\clib OpenCC.egg-info
)

REM Upload to PyPI
REM conda activate py3.8
REM python -m pip install twine
REM python -m twine upload dist/*
