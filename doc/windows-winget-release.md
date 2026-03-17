# Windows Release for WinGet

This repository now includes a Windows release path intended for WinGet distribution.

## Output

Running `scripts/release-windows-winget.ps1` produces:

- `OpenCC-<version>-windows-x64-portable.zip`
- `OpenCC-<version>-windows-x64-portable.zip.sha256`
- `winget-manifests/b/BYVoid/OpenCC/<version>/`

By default the WinGet identity remains `BYVoid.OpenCC`, and the generated `InstallerUrl` points at `https://opencc.byvoid.com/opencc-winget-release`.

The zip is structured as a portable package:

- `bin/*.exe`
- `bin/opencc.dll`
- `share/opencc/*.json`
- `share/opencc/*.ocd2`

OpenCC now also searches `../share/opencc` relative to the running executable or library on Windows, so the portable package works after WinGet extracts it.

The WinGet manifest continues to use `InstallerType: zip` with `NestedInstallerType: portable`. This is intentional: OpenCC is not a single-file executable and still needs `opencc.dll` and `share/opencc/*` alongside the command binaries.

## Directory layout

With the default arguments:

- `BuildDir = build\winget-x64`
- `OutputDir = dist\winget-x64`

the Windows release flow writes files into these locations:

```text
build/
  winget-x64/
    src/
    data/
    test/
    Release/
    Testing/

dist/
  winget-x64/
    OpenCC-<version>-windows-x64-portable.zip
    OpenCC-<version>-windows-x64-portable.zip.sha256
    staging/
      bin/
        opencc.exe
        opencc_dict.exe
        opencc_phrase_extract.exe
        opencc.dll
      share/
        opencc/
          *.json
          *.ocd2
      LICENSE.txt
      README.md
    winget-manifests/
      b/
        BYVoid/
          OpenCC/
            <version>/
              BYVoid.OpenCC.yaml
              BYVoid.OpenCC.installer.yaml
              BYVoid.OpenCC.locale.en-US.yaml
```

The important distinction is:

- `build\winget-x64` is the CMake and test working directory
- `dist\winget-x64\staging\...` is the installed portable tree before zipping
- `dist\winget-x64\OpenCC-<version>-windows-x64-portable.zip` is the final release asset
- `dist\winget-x64\winget-manifests\...` is the manifest payload to copy into `winget-pkgs`

## Local build

Run this from a Visual Studio developer shell or an environment where MSVC is configured:

```powershell
./scripts/release-windows-winget.ps1
```

To build a specific tag or skip tests:

```powershell
./scripts/release-windows-winget.ps1 -Version 1.2.0
./scripts/release-windows-winget.ps1 -Version 1.2.1-rc1
./scripts/release-windows-winget.ps1 -Version 1.2.1-alpha1
./scripts/release-windows-winget.ps1 -Version ver.1.2.0 -SkipTests
```

To test from a fork while keeping the upstream WinGet package identity:

```powershell
./scripts/release-windows-winget.ps1 -Version 1.2.1-rc1 -GitHubRepository frankslin/OpenCC
```

This keeps:

- `PackageIdentifier: BYVoid.OpenCC`
- `manifests/b/BYVoid/OpenCC/<version>/`

but points release URLs at:

- `https://github.com/frankslin/OpenCC`

The public download base URL is fixed to:

```text
https://opencc.byvoid.com/opencc-winget-release
```

Accepted version formats are:

- `x.y.z`
- `x.y.z-rcN`
- `x.y.z-alphaN`

Values such as `1.2.0.exp` are rejected.

## GitHub Actions

The workflow `.github/workflows/release-winget.yml` runs on:

- pushed tags matching `ver.*`
- manual `workflow_dispatch`

On a tag build it will:

1. build the x64 portable Windows release
2. upload the zip and checksum to the GitHub release
3. upload the generated WinGet manifests as a workflow artifact

The workflow passes `${{ github.repository }}` into the release script, so fork builds point to the fork's GitHub release URLs automatically while still using the upstream WinGet package identifier by default.

## WinGet submission

The generated manifests are ready to copy into a `winget-pkgs` checkout under:

```text
manifests/b/BYVoid/OpenCC/<version>/
```

Recommended validation flow in a `winget-pkgs` clone:

```powershell
winget validate manifests\b\BYVoid\OpenCC\<version>
winget install --manifest manifests\b\BYVoid\OpenCC\<version>
```

If you later automate WinGet submission, the generated manifest directory can be used directly as the source payload for a PR bot or `wingetcreate update`.
