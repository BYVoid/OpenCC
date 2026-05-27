@echo off
setlocal EnableDelayedExpansion

python -m pip install --upgrade pip cibuildwheel twine
if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

SET CIBW_BUILD=cp310-* cp311-* cp312-* cp313-* cp314-*
SET CIBW_ARCHS=AMD64
SET CIBW_ENVIRONMENT=VERSION="%VERSION%"
SET CIBW_TEST_REQUIRES=pytest
SET CIBW_TEST_COMMAND=pytest {project}/python/tests

python -m cibuildwheel --output-dir wheelhouse
if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

if NOT "%~1"=="testonly" (
    REM Upload to PyPI
    python -m twine upload wheelhouse/*.whl
)
