@echo off
setlocal EnableDelayedExpansion

python -m pip install --upgrade pip build pytest twine
if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

python -m build --wheel --outdir wheelhouse
if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

for %%f in (wheelhouse\*.whl) do (
    python -m pip install "%%f"
    if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)
)

pytest python/tests
if !ERRORLEVEL! NEQ 0 (EXIT !ERRORLEVEL!)

if NOT "%~1"=="testonly" (
    REM Upload to PyPI
    python -m twine upload wheelhouse/*.whl
)
