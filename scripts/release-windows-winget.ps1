param(
    [string]$Version = "",
    [string]$Arch = "x64",
    [string]$BuildDir = "build\winget-$Arch",
    [string]$OutputDir = "dist\winget-$Arch",
    [string]$GitHubRepository = "BYVoid/OpenCC",
    [string]$PackageIdentifier = "BYVoid.OpenCC",
    [string]$Publisher = "BYVoid",
    [switch]$SkipTests
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-ProjectVersion {
    $packageJsonFile = Join-Path $PSScriptRoot "..\package.json"
    if (Test-Path $packageJsonFile) {
        $packageJson = Get-Content -Path $packageJsonFile -Raw | ConvertFrom-Json
        if ($packageJson.version) {
            return [string]$packageJson.version
        }
    }

    $gitVersionFile = Join-Path $PSScriptRoot "..\cmake\GitVersion.cmake"
    $content = Get-Content -Path $gitVersionFile -Raw

    $major = [regex]::Match($content, '_OPENCC_FALLBACK_MAJOR\s+(\d+)').Groups[1].Value
    $minor = [regex]::Match($content, '_OPENCC_FALLBACK_MINOR\s+(\d+)').Groups[1].Value
    $revision = [regex]::Match($content, '_OPENCC_FALLBACK_REVISION\s+(\d+)').Groups[1].Value

    if (-not $major -or -not $minor -or -not $revision) {
        throw "Failed to parse version from package.json or cmake/GitVersion.cmake."
    }

    return "$major.$minor.$revision"
}

function Write-Utf8File {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Content
    )

    $directory = Split-Path -Parent $Path
    if ($directory) {
        New-Item -ItemType Directory -Force -Path $directory | Out-Null
    }

    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Content, $utf8NoBom)
}

function Normalize-ReleaseVersion {
    param(
        [Parameter(Mandatory = $true)][string]$RawVersion
    )

    $value = $RawVersion.Trim()
    if ($value.StartsWith("ver.")) {
        $value = $value.Substring(4)
    } elseif ($value.StartsWith("v")) {
        $value = $value.Substring(1)
    }

    if ($value -notmatch '^\d+\.\d+\.\d+(-(alpha|rc)\d+)?$') {
        throw "Invalid version '$RawVersion'. Expected x.y.z, x.y.z-alphaN, or x.y.z-rcN."
    }

    return $value
}

function Normalize-GitHubRepository {
    param(
        [Parameter(Mandatory = $true)][string]$RawRepository
    )

    $value = $RawRepository.Trim()
    if ($value -notmatch '^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$') {
        throw "Invalid GitHub repository '$RawRepository'. Expected owner/name."
    }
    return $value
}

function Normalize-BaseUrl {
    param(
        [Parameter(Mandatory = $true)][string]$RawUrl
    )

    $value = $RawUrl.Trim().TrimEnd('/')
    if ($value -notmatch '^https://') {
        throw "Invalid public base URL '$RawUrl'. Expected an https URL."
    }
    return $value
}

function New-WinGetManifests {
    param(
        [Parameter(Mandatory = $true)][string]$PackageVersion,
        [Parameter(Mandatory = $true)][string]$PackageUrl,
        [Parameter(Mandatory = $true)][string]$InstallerSha256,
        [Parameter(Mandatory = $true)][string]$ManifestRoot,
        [Parameter(Mandatory = $true)][string]$PackageIdentifier,
        [Parameter(Mandatory = $true)][string]$Publisher,
        [Parameter(Mandatory = $true)][string]$PublisherUrl,
        [Parameter(Mandatory = $true)][string]$PublisherSupportUrl,
        [Parameter(Mandatory = $true)][string]$PackagePageUrl,
        [Parameter(Mandatory = $true)][string]$LicenseUrl
    )
    $releaseDate = Get-Date -Format "yyyy-MM-dd"

    $versionManifest = @"
# yaml-language-server: `$schema=https://aka.ms/winget-manifest.version.1.9.0.schema.json
PackageIdentifier: $packageIdentifier
PackageVersion: $PackageVersion
DefaultLocale: en-US
ManifestType: version
ManifestVersion: 1.9.0
"@

    $installerManifest = @"
# yaml-language-server: `$schema=https://aka.ms/winget-manifest.installer.1.9.0.schema.json
PackageIdentifier: $packageIdentifier
PackageVersion: $PackageVersion
InstallerType: zip
NestedInstallerType: portable
ReleaseDate: $releaseDate
Installers:
- Architecture: x64
  InstallerUrl: $PackageUrl
  InstallerSha256: $InstallerSha256
  NestedInstallerFiles:
  - RelativeFilePath: bin/opencc.exe
    PortableCommandAlias: opencc
  - RelativeFilePath: bin/opencc_dict.exe
    PortableCommandAlias: opencc_dict
  - RelativeFilePath: bin/opencc_phrase_extract.exe
    PortableCommandAlias: opencc_phrase_extract
ManifestType: installer
ManifestVersion: 1.9.0
"@

    $localeManifest = @"
# yaml-language-server: `$schema=https://aka.ms/winget-manifest.defaultLocale.1.9.0.schema.json
PackageIdentifier: $packageIdentifier
PackageVersion: $PackageVersion
PackageLocale: en-US
Publisher: $Publisher
PublisherUrl: $PublisherUrl
PublisherSupportUrl: $PublisherSupportUrl
Author: Carbo Kuo and OpenCC contributors
PackageName: OpenCC
PackageUrl: $PackagePageUrl
License: Apache-2.0
LicenseUrl: $LicenseUrl
ShortDescription: Open Chinese Convert
Description: Command-line tools and libraries for Simplified Chinese, Traditional Chinese, regional variant, and Japanese Kanji conversion.
Moniker: opencc
Tags:
- chinese
- conversion
- opencc
- simplified-chinese
- traditional-chinese
ManifestType: defaultLocale
ManifestVersion: 1.9.0
"@

    Write-Utf8File -Path (Join-Path $ManifestRoot "BYVoid.OpenCC.yaml") -Content $versionManifest
    Write-Utf8File -Path (Join-Path $ManifestRoot "BYVoid.OpenCC.installer.yaml") -Content $installerManifest
    Write-Utf8File -Path (Join-Path $ManifestRoot "BYVoid.OpenCC.locale.en-US.yaml") -Content $localeManifest
}

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
Push-Location $repoRoot

try {
    $publicBaseUrl = "https://opencc.byvoid.com/opencc-winget-release"

    if (-not $Version) {
        $Version = Get-ProjectVersion
    }

    $Version = Normalize-ReleaseVersion -RawVersion $Version
    $GitHubRepository = Normalize-GitHubRepository -RawRepository $GitHubRepository

    if ($Arch -ne "x64") {
        throw "This script currently supports only -Arch x64 for WinGet releases."
    }

    $packageRoot = "https://github.com/$GitHubRepository"
    $releaseUrlBase = Normalize-BaseUrl -RawUrl $publicBaseUrl
    $licenseUrl = "$packageRoot/blob/master/LICENSE"
    $publisherSupportUrl = "$packageRoot/issues"

    $resolvedBuildDir = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $BuildDir))
    $resolvedOutputDir = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $OutputDir))
    $installRoot = Join-Path $resolvedOutputDir "install"
    $stagingRoot = Join-Path $resolvedOutputDir "staging"
    $assetName = "OpenCC-$Version-windows-$Arch-portable.zip"
    $assetPath = Join-Path $resolvedOutputDir $assetName
    $checksumPath = "$assetPath.sha256"
    $wingetPathIdentifier = $PackageIdentifier.Replace('.', '\')
    $wingetRoot = Join-Path $resolvedOutputDir "winget-manifests\$wingetPathIdentifier\$Version"
    $releaseUrl = "$releaseUrlBase/$assetName"

    Remove-Item -Recurse -Force $resolvedBuildDir -ErrorAction SilentlyContinue
    Remove-Item -Recurse -Force $resolvedOutputDir -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $resolvedBuildDir | Out-Null
    New-Item -ItemType Directory -Force -Path $installRoot | Out-Null
    New-Item -ItemType Directory -Force -Path $stagingRoot | Out-Null

    cmake -S . -B $resolvedBuildDir -A x64 `
        -DCMAKE_BUILD_TYPE=Release `
        -DBUILD_SHARED_LIBS:BOOL=OFF `
        -DBUILD_OPENCC_JIEBA_PLUGIN:BOOL=ON `
        -DCMAKE_INSTALL_PREFIX:PATH=$installRoot `
        -DENABLE_GTEST:BOOL=OFF `
        -DENABLE_BENCHMARK:BOOL=OFF

    cmake --build $resolvedBuildDir --config Release --target install

    if (-not $SkipTests) {
        ctest --test-dir $resolvedBuildDir --build-config Release --output-on-failure
    }

    New-Item -ItemType Directory -Force -Path (Join-Path $stagingRoot "bin") | Out-Null
    New-Item -ItemType Directory -Force -Path (Join-Path $stagingRoot "bin\plugins") | Out-Null
    New-Item -ItemType Directory -Force -Path (Join-Path $stagingRoot "share\opencc") | Out-Null

    Copy-Item -Path (Join-Path $installRoot "bin\opencc.exe") -Destination (Join-Path $stagingRoot "bin\opencc.exe")
    Copy-Item -Path (Join-Path $installRoot "bin\opencc_dict.exe") -Destination (Join-Path $stagingRoot "bin\opencc_dict.exe")
    Copy-Item -Path (Join-Path $installRoot "bin\opencc_phrase_extract.exe") -Destination (Join-Path $stagingRoot "bin\opencc_phrase_extract.exe")
    Copy-Item -Path (Join-Path $installRoot "bin\plugins\*.dll") -Destination (Join-Path $stagingRoot "bin\plugins")
    Copy-Item -Path (Join-Path $installRoot "share\opencc\*") -Destination (Join-Path $stagingRoot "share\opencc") -Recurse
    Copy-Item -Path LICENSE -Destination (Join-Path $stagingRoot "LICENSE.txt")
    Copy-Item -Path README.md -Destination (Join-Path $stagingRoot "README.md")

    Compress-Archive -Path (Join-Path $stagingRoot '*') -DestinationPath $assetPath -CompressionLevel Optimal

    $hash = (Get-FileHash -Path $assetPath -Algorithm SHA256).Hash.ToUpperInvariant()
    Write-Utf8File -Path $checksumPath -Content "$hash *$assetName`n"

    New-WinGetManifests `
        -PackageVersion $Version `
        -PackageUrl $releaseUrl `
        -InstallerSha256 $hash `
        -ManifestRoot $wingetRoot `
        -PackageIdentifier $PackageIdentifier `
        -Publisher $Publisher `
        -PublisherUrl $packageRoot `
        -PublisherSupportUrl $publisherSupportUrl `
        -PackagePageUrl $packageRoot `
        -LicenseUrl $licenseUrl

    Write-Host "Release archive: $assetPath"
    Write-Host "SHA256: $hash"
    Write-Host "WinGet manifests: $wingetRoot"
}
finally {
    Pop-Location
}
