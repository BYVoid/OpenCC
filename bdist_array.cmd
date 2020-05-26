@echo off
setlocal EnableDelayedExpansion

SET VERSIONS=2.7 3.5 3.6 3.7 3.8
SET ROOT_DIR=%cd%

for %%v in (%VERSIONS%) do (
    echo Building for python==%%v
    SET ENV_NAME=opencc-%%v

    REM Clean up existing builds
    cd %ROOT_DIR%
    rmdir /S /Q build python\opencc\clib OpenCC.egg-info

    REM Create and initialize new environment
    cd %ROOT_DIR%
    CALL conda create -y -n !ENV_NAME! python=%%v
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
    CALL conda activate !ENV_NAME!
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

    pip install --no-cache-dir setuptools wheel pytest
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

    REM Build install and test
    cd %ROOT_DIR%
    python setup.py build_ext install
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
    cd %ROOT_DIR%\python
    pytest .
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

    REM Package and clean up
    cd %ROOT_DIR%
    del dist\*.egg
    python setup.py bdist_wheel
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
    rmdir /S /Q build python\opencc\clib OpenCC.egg-info
    CALL conda deactivate

    echo Finished building for python==%%v
)
