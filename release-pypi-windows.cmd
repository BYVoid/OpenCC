@echo off
setlocal EnableDelayedExpansion

SET VERSIONS=3.8 3.9 3.10 3.11 3.12
SET SOURCEDIR=%cd%

REM Accept conda Terms of Service for required channels
CALL C:\Miniconda/condabin/conda.bat tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main
CALL C:\Miniconda/condabin/conda.bat tos accept --override-channels --channel https://repo.anaconda.com/pkgs/r

REM Build packages
for %%v in (%VERSIONS%) do (
    SET ENV_NAME=py%%v

    REM Create and activate environment
    cd %ROOT_DIR%
    CALL C:\Miniconda/condabin/conda.bat config --add channels conda-forge
    CALL C:\Miniconda/condabin/conda.bat config --set channel_priority strict

    CALL C:\Miniconda/condabin/conda.bat create -y -n py%%v python=%%v
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
    CALL C:\Miniconda/condabin/conda.bat activate py%%v
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
    pip install --no-cache-dir build
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

    REM Build and package
    python -m build --wheel
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

    REM Cleanup
    CALL C:\Miniconda/condabin/conda.bat deactivate
    rmdir /S /Q build OpenCC.egg-info
)

if NOT "%~1"=="testonly" (
    REM Upload to PyPI
    C:\Miniconda/condabin/conda.bat activate py3.8
    python -m pip install twine
    python -m twine upload dist/*
)
